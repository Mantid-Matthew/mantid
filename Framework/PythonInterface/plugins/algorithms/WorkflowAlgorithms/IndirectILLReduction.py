#pylint: disable=no-init,invalid-name,too-many-instance-attributes
from __future__ import (absolute_import, division, print_function)

from mantid.simpleapi import *
from mantid.kernel import StringListValidator, IntBoundedValidator, Direction
from mantid.api import DataProcessorAlgorithm, PropertyMode,\
	AlgorithmFactory, FileProperty, FileAction, \
	MatrixWorkspaceProperty, MultipleFileProperty, \
	Progress, WorkspaceGroup
from mantid import config, logger, mtd
#from scipy.constants import codata, Planck # needed eventually for energy computation

import numpy as np
import os.path

"""
ws          : workspace
det_grouped : grouping of detectors and not a WorkspaceGroup (_workspace_group)
mnorm       : monitor normalised
vnorm       : vanadium normalised
red         : reduced

In the following, ws as function parameter refers to the input workspace whereas ws_out refers to the output workspace
"""

class IndirectILLReduction(DataProcessorAlgorithm):

    # Workspaces
    _raw_ws = None
    _det_grouped_ws = None
    _monitor_ws = None
    _mnorm_ws = None
    _vnorm_ws = None
    _red_ws = None
    _red_left_ws = None
    _red_right_ws = None
    _calibration_ws = None
    _vanadium_ws = None

    # Files
    _map_file = None
    _run_file = None
    _vanadium_run = None
    _parameter_file = None

    # Bool flags
    _control_mode = None
    _sum_runs = None
    _save = None
    _plot = None
    _mirror_sense = None

    # Integer
    _unmirror_option = 3

    # Other
    _instrument_name = None
    _instrument = None
    _analyser = None
    _reflection = None

    def category(self):
        return "Workflow\\MIDAS;Inelastic\\Reduction"

    def summary(self):
        return 'Performs an energy transfer reduction for ILL indirect inelastic data, instrument IN16B.'

    def PyInit(self):
        # Input options
        # This has to be MultipleFileProperty.
        self.declareProperty(MultipleFileProperty('Run',
                extensions=["nxs"]),
                doc='File path of run (s).')

        self.declareProperty(name='Analyser',
                defaultValue='silicon',
                validator=StringListValidator(['silicon']),
                doc='Analyser crystal.')

        self.declareProperty(name='Reflection',
                defaultValue='111',
                validator=StringListValidator(['111']),
                doc='Analyser reflection.')

        self.declareProperty(FileProperty('MapFile',
                '',
                action=FileAction.OptionalLoad,
                extensions=["xml"]),
                doc='Filename of the map file to use. If left blank the default will be used.')

        # Output workspace properties
        self.declareProperty(MatrixWorkspaceProperty("ReducedWorkspace",
                "red",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the output reduced workspace created. \n Depending on the unmirror option.")

        self.declareProperty(MatrixWorkspaceProperty("CalibrationWorkspace",
                "",
                optional=PropertyMode.Optional,
                direction=Direction.Input),
                doc="Workspace containing calibration intensities.")

        self.declareProperty(name='ControlMode',
                defaultValue=False,
                doc='Whether to output the workspaces in intermediate steps.')

        self.declareProperty(name='SumRuns',
                defaultValue=False,
                doc='Whether to sum all the input runs.')

        self.declareProperty(name='MirrorSense',
                defaultValue=True,
                doc='Whether the input data has two wings.')

        self.declareProperty(name='UnmirrorOption',
                defaultValue=3,
                validator=IntBoundedValidator(lower=0,upper=7),
                doc='Unmirroring options: \n 0 Normalisation of grouped workspace to monitor spectrum and no energy transfer\n 1 left workspace\n 2 right workspace\n 3 sum of left and right workspaces\n 4 shift right workspace according to maximum peak positions of left workspace \n 5 like option 4 and center all spectra \n 6 like 4, but use Vanadium workspace for estimating maximum peak positions \n 7 like 6 and center all spectra')

        # Optional workspaces created when running in debug mode
        self.declareProperty(MatrixWorkspaceProperty("RawWorkspace",
                "raw",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the output raw workspace created.")

        self.declareProperty(MatrixWorkspaceProperty("MNormalisedWorkspace",
                "mnorm",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the workspace normalised to monitor.")

        self.declareProperty(MatrixWorkspaceProperty("DetGroupedWorkspace",
                "detectors_grouped",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the workspace with grouped detectors.")

        self.declareProperty(MatrixWorkspaceProperty("MonitorWorkspace",
                "monitor",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the monitor spectrum.")

        self.declareProperty(MatrixWorkspaceProperty("VNormalisedWorkspace",
                "vnorm",
                optional=PropertyMode.Optional,
                direction=Direction.Output),
                doc="Name for the workspace normalised to vanadium.")

        # Output options
        self.declareProperty(name='Save',
                defaultValue=False,
                doc='Whether to save the output workpsaces to nxs file.')
        self.declareProperty(name='Plot',
                defaultValue=False,
                doc='Whether to plot the output workspace.')

    def validateInputs(self):

        issues = dict()
        # Unmirror options 6 and 7 require a Vanadium run as input workspace
        if self._mirror_sense and self._unmirror_option==6 and self._vanadium_run!='':
            issues['UnmirrorOption'] = 'Unmirror option 6 requires calibration workspace to be set'
        if self._mirror_sense and self._unmirror_option==7 and self._vanadium_run!='':
            issues['UnmirrorOption'] = 'Unmirror option 7 requires calibration workspace to be set'

        return issues

    def setUp(self):

        self._run_file = self.getPropertyValue('Run')

        self._raw_ws = self.getPropertyValue('RawWorkspace')
        self._det_grouped_ws = self.getPropertyValue('DetGroupedWorkspace')
        self._red_ws= self.getPropertyValue('ReducedWorkspace')
        self._calibration_ws = self.getPropertyValue('CalibrationWorkspace')
        self._vnorm_ws = self.getPropertyValue('VNormalisedWorkspace')
        self._mnorm_ws = self.getPropertyValue('MNormalisedWorkspace')
        self._monitor_ws = self.getPropertyValue('MonitorWorkspace')

        self._analyser = self.getPropertyValue('Analyser')
        self._map_file = self.getPropertyValue('MapFile')
        self._reflection = self.getPropertyValue('Reflection')

        self._mirror_sense = self.getProperty('MirrorSense').value
        self._control_mode = self.getProperty('ControlMode').value
        self._plot = self.getProperty('Plot').value
        self._save = self.getProperty('Save').value
        self._sum_runs = self.getProperty('SumRuns').value
        self._unmirror_option = self.getProperty('UnmirrorOption').value

        if not self._mirror_sense:
            self.log().warning('MirrorSense is OFF, UnmirrorOption will fall back to 0 (i.e. no unmirroring)')

        self.validateInputs()

    def PyExec(self):

        self.setUp()

        # if SumAll is specified, force to sum all the listed files
        if self._sum_runs:
            self._run_file = self._run_file.replace(',','+')

        # This must be Load, to be able to treat multiple files
        Load(Filename=self._run_file, OutputWorkspace=self._raw_ws)
        print('Loaded .nxs file(s) : %s' % self._run_file)
        # When multiple files are loaded, raw_ws will be a group workspace
        # containing workspaces having run numbers as names

        if isinstance(mtd[self._raw_ws],WorkspaceGroup):
            # Get instrument from the first ws in a group
            self._instrument = mtd[self._raw_ws].getItem(0).getInstrument()
            self._load_config_files()
            runlist = []
            for i in range(0, mtd[self._raw_ws].size()):
                run = str(mtd[self._raw_ws].getItem(i).getRunNumber())
                runlist.append(run)
                ws = run + '_' + self._raw_ws
                RenameWorkspace(InputWorkspace = run, OutputWorkspace = ws)
                self._reduce_run(run)

            # after all the runs are reduced, set output ws
            self._set_output_workspace_properties(runlist)
        else:
            self._instrument = mtd[self._raw_ws].getInstrument()
            run = str(mtd[self._raw_ws].getRunNumber())
            self._load_config_files()
            # After loading sinlge file, it will have "raw" name
            # So rename to RunNumber_raw for consistency with multiple file case
            ws = run + '_' + self._raw_ws
            RenameWorkspace(InputWorkspace = self._raw_ws, OutputWorkspace = ws)
            self._reduce_run(run)
            # after reduction, set output ws
            self._set_output_workspace_properties([run])

    def _load_config_files(self):
        """
        Loads parameter and detector grouping map file
        """

        print('Load instrument definition file and detectors grouping (map) file')
        self._instrument_name = self._instrument.getName()
        self._analyser = self.getPropertyValue('Analyser')
        self._reflection = self.getPropertyValue('Reflection')

        idf_directory = config['instrumentDefinition.directory']
        ipf_name = self._instrument_name + '_' + self._analyser + '_' + self._reflection + '_Parameters.xml'
        self._parameter_file = os.path.join(idf_directory, ipf_name)

        print('Parameter file : %s' % self._parameter_file)

        if self._map_file == '':
            # path name for default map file
            if self._instrument.hasParameter('Workflow.GroupingFile'):
               grouping_filename = self._instrument.getStringParameter('Workflow.GroupingFile')[0]
               self._map_file = os.path.join(config['groupingFiles.directory'], grouping_filename)
            else:
               raise ValueError("Failed to find default detector grouping file. Please specify manually.")

        print('Map file : %s' % self._map_file)
        print('Done')

    def _reduce_run(self, run):
        """
        Run indirect reduction for IN16B
        @ param         run :: string of run number to reduce
        @ return         :: the reduced workspace or a list of workspaces
                           depending on the control mode
        """

        # Must use named temporaries here
        raw_ws = run + '_' + self._raw_ws
        det_grouped_ws = run + '_' + self._det_grouped_ws
        monitor_ws = run + '_' + self._monitor_ws
        red_ws = run + '_' + self._red_ws
        mnorm_ws = run + '_' + self._mnorm_ws
        vnorm_ws = run + '_' + self._vnorm_ws


        print('Reduction')
        print('Input workspace : %s' % raw_ws)


        # Raw workspace
        LoadParameterFile(Workspace=raw_ws,Filename=self._parameter_file)

        # Detectors grouped workspace
        GroupDetectors(InputWorkspace=raw_ws,
                                OutputWorkspace=det_grouped_ws,
                                MapFile=self._map_file,
                                Behaviour='Sum')

        # Monitor workspace
        ExtractSingleSpectrum(InputWorkspace=raw_ws,
                                OutputWorkspace=monitor_ws,
                                WorkspaceIndex=0)

        self._unmirror(run)

        print('Add unmirror_option and _sum_runs to Sample Log of reduced workspace (right click on workspace, click on Sample Logs...)')
        #AddSampleLog(Workspace=self._red_ws, LogName="unmirror_option",
        #                        LogType="String", LogText=str(self._unmirror_option))
        #AddSampleLog(Workspace=self._red_ws, LogName="_sum_runs",
        #                        LogType="String", LogText=str(self._sum_runs))
        print('Done')

    def _unmirror(self, run):
        """
        Runs energy reduction for corresponding unmirror option.

        This function calls self._calculate_energy() for the energy transfer

        """
        print('Unmirror')

        # Again need temporaries here
        raw_ws = run + '_' + self._raw_ws
        det_grouped_ws = run + '_' + self._det_grouped_ws
        monitor_ws = run + '_' + self._monitor_ws
        red_ws = run + '_' + self._red_ws
        mnorm_ws = run + '_' + self._mnorm_ws
        vnorm_ws = run + '_' + self._vnorm_ws

        if self._unmirror_option == 0:
            print('Normalisation of grouped workspace to monitor, bins will not be masked, X-axis will not be in energy transfer.')
            # No energy transform is performed, since
            # energy transfer requires X-Axis to be in range -Emin Emax -Emin Emax ...

            NormaliseToMonitor(InputWorkspace=det_grouped_ws,
                            MonitorWorkspace=monitor_ws,
                            OutputWorkspace=mnorm_ws)

            CloneWorkspace(Inputworkspace=mnorm_ws, OutputWorkspace=red_ws)

        else:
            # Get X-values of the grouped workspace
            x = mtd[det_grouped_ws].readX(0)
            x_end = len(x)
            mid_point = int((x_end - 1) / 2)

            if self._unmirror_option == 1:
                print('Left reduced, monitor, detectors grouped and normalised workspace')
                self._extract_workspace(0, x[mid_point], red_ws, monitor_ws, det_grouped_ws, mnorm_ws, run)

            if self._unmirror_option == 2:
                print('Right reduced, monitor, detectors grouped and normalised workspace')
                self._extract_workspace(x[mid_point], x_end, red_ws, monitor_ws, det_grouped_ws, mnorm_ws, run)

            if self._unmirror_option > 2:
                # Temporary workspace names needed for unmirror options 3 to 7
                __left_ws = '__left_ws'
                __left_monitor_ws = '__left_monitor_ws'
                __left_grouped_ws = '__left_grouped_ws'
                __left_mnorm_ws = '__left_mnorm_ws'
                __right_ws = '__right_ws'
                __right_monitor_ws = '__right_monitor_ws'
                __right_grouped_ws = '__right_grouped_ws'
                __right_mnorm_ws = '__right_mnorm_ws'

                __left = '__right'
                __right = '__left'

                # Left workspace
                self._extract_workspace(0,
                                                                x[mid_point],
                                                                __left_ws,
                                                                __left_monitor_ws,
                                                                __left_grouped_ws,
                                                                __left_mnorm_ws,
                                                                run)

                # Right workspace
                self._extract_workspace(x[mid_point],
                                                                x_end,
                                                                __right_ws,
                                                                __right_monitor_ws,
                                                                __right_grouped_ws,
                                                                __right_mnorm_ws,
                                                                run)

                if self._unmirror_option == 3:
                    print('Sum left and right workspace for unmirror option 3')
                    __left = __left_ws
                    __right = __right_ws
                elif self._unmirror_option == 4:
                    print('Shift each sepctrum of the right workspace according to the maximum peak positions of the corresponding spectrum of the left workspace')
                    self._shift_spectra(__right_ws, __left_ws, __right)
                    __left = __left_ws
                    # Shifted workspace in control mode?
                elif self._unmirror_option == 5:
                    # _shift_spectra needs extension
                    pass
                elif self._unmirror_option == 6:
                    # Vanadium file must be loaded, left and right workspaces extracted
                    # Update PyExec and _set_workspace_properties accordingly, _vanadium_ws = None
                    pass
                elif self._unmirror_option == 7:
                    # Vanadium file must be loaded, left and right workspaces extracted
                    # Update PyExec and _set_workspace_properties accordingly, _vanadium_ws = None
                    pass

                # Sum left, right and corresponding workspaces
                self._perform_mirror(__left, __right, run,
                                                        __left_monitor_ws,
                                                        __right_monitor_ws,
                                                        __left_grouped_ws,
                                                        __right_grouped_ws,
                                                        __left_mnorm_ws,
                                                        __right_mnorm_ws)

        print('Done')

    def _calculate_energy(self, ws, ws_out, run):
        """
        Convert the input run to energy transfer
        @param ws       :: energy transfer for workspace ws
                @param ws_out   :: output workspace
        """

        print('Energy calculation')

        # Again temporaries needed
        vnorm_ws = run + '_' + self._vnorm_ws

        # Apply the detector intensity calibration
        if self._calibration_ws != '':
            Divide(LHSWorkspace=ws,
                            RHSWorkspace=self._calibration_ws,
                            OutputWorkspace=vnorm_ws)
        else:
            CloneWorkspace(InputWorkspace=ws,
                            OutputWorkspace=vnorm_ws)

        formula = self._energy_range(vnorm_ws)

        ConvertAxisByFormula(InputWorkspace=vnorm_ws,
                                OutputWorkspace=ws_out,
                                Axis='X',
                                Formula=formula)

        # Set unit of the X-Axis
        mtd[ws_out].getAxis(0).setUnit('DeltaE')

        xnew = mtd[ws_out].readX(0)  # energy array
        print('Energy range : %f to %f' % (xnew[0], xnew[-1]))

        print('Done')

    def _energy_range(self, ws):
        """
        Calculate the energy range for the workspace

        @param ws :: name of the input workspace
        @return   :: formula to transform from time channel to energy transfer
        """
        print('Calculate energy range')

        x = mtd[ws].readX(0)
        npt = len(x)
        imid = float( npt / 2 + 1 )
        gRun = mtd[ws].getRun()
        energy = 0

        if gRun.hasProperty('Doppler.maximum_delta_energy'):
            # Check whether Doppler drive has incident_wavelength
            velocity_profile = gRun.getLogData('Doppler.velocity_profile').value
            if velocity_profile == 0:
                energy = gRun.getLogData('Doppler.maximum_delta_energy').value  # max energy in meV
                print('Doppler max energy : %s' % energy)
            else:
                print('Check operation mode of Doppler drive: velocity profile 0 (sinudoidal) required')
        elif gRun.hasProperty('Doppler.delta_energy'):
            energy = gRun.getLogData('Doppler.delta_energy').value  # delta energy in meV
            print('Warning: Doppler delta energy used : %s' % energy)
        else:
            print('Error: Energy is 0 micro electron Volt')

        dele = 2.0 * energy / (npt - 1)
        formula = '(x-%f)*%f' % (imid, dele)

        print('Done')
        return formula

    def _monitor_range(self, monitor_ws):
        """
        Get sensible values for the min and max cropping range
        @param monitor_ws :: name of the monitor workspace
        @return tuple containing the min and max x values in the range
        """
        print('Determine monitor range')

        x = mtd[monitor_ws].readX(0)  # energy array
        y = mtd[monitor_ws].readY(0)  # energy array

        # mid x value in order to seach for first and right monitor range delimiter
        mid = int(len(x) / 2)

        imin = np.argmax(np.array(y[0 : mid])) - 1
        nch = len(y)
        im = np.argmax(np.array(y[nch - mid : nch]))
        imax = nch - mid + 1 + im + 1

        print('Cropping range %f to %f' % (x[imin], x[imax]))
        print('Cropping range %f to %f' % (x[imin], x[imax]))

        print('Done')
        return x[imin], x[imax]

    def _extract_workspace(self, x_start, x_end, ws_out, monitor, det_grouped, mnorm, run):
        """
        Extract left or right workspace from detector grouped workspace and perform energy transfer
        @params      :: x_start defines start bin of workspace to be extracted
        @params      :: x_end defines end bin of workspace to be extracted
        @params      :: x_shift determines shift of x-values
        @return      :: ws_out reduced workspace, normalised, bins masked, energy transfer
        @return      :: corresponding monitor spectrum
        @return      :: corresponding detector grouped workspace
        @return      :: corresponding normalised workspace
        """
        print('Extract left or right workspace from detector grouped workspace and perform energy transfer')

        # named temporaries (do not use self.* directly, neither overwrite,
        # otherwise multiple file reduction will break down
        det_grouped_in = run + '_' + self._det_grouped_ws
        monitor_in = run + '_' + self._monitor_ws

        CropWorkspace(InputWorkspace=det_grouped_in,
                                        OutputWorkspace=det_grouped,
                                        XMin=x_start,
                                        XMax=x_end)

        CropWorkspace(InputWorkspace=monitor_in,
                                        OutputWorkspace=monitor,
                                        XMin=x_start,
                                        XMax=x_end)

        # Shift X-values of extracted workspace and its corresponding monitor in order to let X-values start with 0
        __x = mtd[det_grouped].readX(0)
        x_shift =  - __x[0]

        ScaleX(InputWorkspace=det_grouped,
                                        OutputWorkspace=det_grouped,
                                        Factor=x_shift,
                                        Operation='Add')

        ScaleX(InputWorkspace=monitor,
                                        OutputWorkspace=monitor,
                                        Factor=x_shift,
                                        Operation='Add')

        # Normalise left workspace to left monitor spectrum
        NormaliseToMonitor(InputWorkspace=det_grouped,
                                OutputWorkspace=mnorm,
                                MonitorWorkspace=monitor)

        # Division by zero for bins that will be masked since MaskBins does not take care of them
        ReplaceSpecialValues(InputWorkspace=mnorm,
                                                OutputWorkspace=mnorm,
                                                NaNValue=0)

        # Mask bins according to monitor range
        xmin, xmax = self._monitor_range(monitor)

        # MaskBins cannot mask nan values!
        # Mask bins (first bins) outside monitor range (nan's after normalisation)
        MaskBins(InputWorkspace=mnorm,
                                                OutputWorkspace=mnorm,
                                                XMin=0,
                                                XMax=xmin)

        # Mask bins (last bins)  outside monitor range (nan's after normalisation)
        MaskBins(InputWorkspace=mnorm,
                                                OutputWorkspace=mnorm,
                                                XMin=xmax,
                                                XMax=x_end)

        self._calculate_energy(mnorm, ws_out, run)

        print('Done')

    def _perform_mirror(self, ws1, ws2, run, monitor1=None, monitor2=None, detgrouped1=None, detgrouped2=None, mnorm1=None, mnorm2=None):
        """
        @params    :: ws1 reduced left workspace
        @params    :: ws2 reduced right workspace
        @params    :: monitor1 optional left monitor workspace
        @params    :: monitor2 optional right monitor workspace
        @params    :: detgrouped1 optional left detectors grouped workspace
        @params    :: detgrouped2 optional right detectors grouped workspace
        @params    :: mnorm1 optional left normalised workspace
        @params    :: mnorm2 optional right normalised workspace
        """

        # named temporaries
        red = run + '_' + self._red_ws
        mon = run + '_' + self._monitor_ws
        det_grouped = run + '_' + self._det_grouped_ws
        mnorm = run + '_' + self._mnorm_ws

        # Reduced workspaces
        Plus(LHSWorkspace=ws1,
                RHSWorkspace=ws2,
                OutputWorkspace=red)
        Scale(InputWorkspace=red,
                OutputWorkspace=red,
                Factor=0.5, Operation='Multiply')

        if (monitor1 is not None and monitor2 is not None
           and detgrouped1 is not None and detgrouped2 is not None
           and mnorm1 is not None and mnorm2 is not None):
            # Monitors
            Plus(LHSWorkspace=monitor1,
                    RHSWorkspace=monitor2,
                    OutputWorkspace=mon)
            Scale(InputWorkspace=mon,
                    OutputWorkspace=mon,
                    Factor=0.5, Operation='Multiply')

            # detectors grouped workspaces
            Plus(LHSWorkspace=detgrouped1,
                    RHSWorkspace=detgrouped2,
                    OutputWorkspace=det_grouped )
            Scale(InputWorkspace=det_grouped ,
                    OutputWorkspace=det_grouped ,
                    Factor=0.5, Operation='Multiply')

            # Normalised workspaces
            Plus(LHSWorkspace=mnorm1,
                    RHSWorkspace=mnorm2,
                    OutputWorkspace=mnorm)
            Scale(InputWorkspace=mnorm,
                    OutputWorkspace=mnorm,
                    Factor=0.5, Operation='Multiply')

    def _shift_spectra(self, ws, ws_shift_origin, ws_out):
        """
        @params   :: ws workspace to be shifted
        @params   :: ws_shift_origin that provides the spectra according to which the shift will be performed
        @params   :: ws_out shifted output workspace

        """
        print('Shift spectra')

        # Get a table maximum peak positions via optimization with Gaussian
        # FindEPP needs modification: validator for TOF axis disable and category generalisation
        table_shift = FindEPP(InputWorkspace=ws_shift_origin)

        number_spectra = mtd[ws].getNumberHistograms()

        # Shift each single spectrum
        for i in range(number_spectra):
            print('Process spectrum ' + str(i))

            __temp = ExtractSingleSpectrum(InputWorkspace=ws, WorkspaceIndex=i)
            peak_position = int(table_shift.row(i)["PeakCentre"])

            x_values = np.array(__temp.readX(0)) # will not change
            y_values = np.array(__temp.readY(0)) # will be shifted
            e_values = np.array(__temp.readE(0)) # will be shifted

            # Perform shift only if FindEPP returns success. Possibility to use self._peak_maximum_position() instead
            if peak_position:

                # A proposition was to use ConvertAxisByFormula. I my opinion the code would be much longer
                # This is the implementation of a circular shift
                if peak_position < 0:
                    # Shift to the right
                    # y-values, e-values : insert last values at the beginning
                    for k in range(peak_position):
                        y_values = np.insert(y_values, 0, y_values[-1])
                        y_values = np.delete(y_values, -1)
                        e_values = np.insert(e_values, 0, e_values[-1])
                        e_values = np.delete(e_values, -1)
                else:
                    # Shift to the left
                    # y-values, e-values : insert last values at the beginning
                    for k in range(peak_position):
                        y_values = np.append(y_values, y_values[0])
                        y_values = np.delete(y_values, 0)
                        e_values = np.append(e_values, e_values[0])
                        e_values = np.delete(e_values, 0)

            if not i:
                # Initial spectrum 0
                CreateWorkspace(OutputWorkspace=ws_out, DataX=x_values, DataY=y_values, DataE=e_values, NSpec=1)#UnitX='TOF'
                # Mask shifted bins missing here
            else:
                # Override temporary workspace __temp by ith shifted spectrum and append to output workspace
                __temp_single_spectrum = CreateWorkspace(DataX=x_values, DataY=y_values, DataE=e_values, NSpec=1)
                # Mask shifted bins missing here
                AppendSpectra(InputWorkspace1=ws_out,
                                                InputWorkspace2=__temp_single_spectrum,
                                                OutputWorkspace=ws_out)

        print('Done')

    def _peak_maximum_position(self, ws):
        """
        @return    :: position where peak of single spectrum has its maximum
        """
        print('Get (reasonable) position at maximum peak value if FindEPP cannot determine it (peak too narrow)')

        # Consider the normalised workspace
        x_values = np.array(mtd[ws].readX(0))
        y_values = np.array(mtd[ws].readY(0))

        y_imax = np.argmax(y_values)

        maximum_position = x_values[y_imax]

        bin_range = int(len(x_values) / 4)
        mid = int(len(x_values) / 2)

        if maximum_position in range(mid - bin_range, mid + bin_range):
                print('Maybe no peak present, take mid position instead (no shift operation of this spectrum)')
                maximum_position = mid

        print('Done')

        return maximum_position

    def _set_output_workspace_properties(self, runlist):
        print('Set workspace properties and deletes temporary workspaces depending on control mode.')

        if len(runlist) > 1:
            # multiple runs
            list_red = []

            # first group and set reduced ws
            for run in runlist:
                list_red.append(run + '_' + self._red_ws)
            GroupWorkspaces(list_red, OutputWorkspace=self._red_ws)
            self.setPropertyValue('ReducedWorkspace', self._red_ws)

            # plot and save if needed, before deleting
            if self._save:
                self._save_reduced_ws(self._red_ws)

            if self._plot:
                self.log().warning('Automatic plotting for multiple files is disabled.')
                print('Automatic plotting for multiple files is disabled.')

            if not self._control_mode:
                # delete everything else
                for run in runlist:
                    DeleteWorkspace(run + '_' + self._raw_ws)
                    DeleteWorkspace(run + '_' + self._mnorm_ws)
                    DeleteWorkspace(run + '_' + self._det_grouped_ws)
                    DeleteWorkspace(run + '_' + self._monitor_ws)
                    if self._unmirror_option > 0 :
                        DeleteWorkspace(run + '_' + self._vnorm_ws)
            else:
                # group and set optional properties
                list_raw = []
                list_monitor = []
                list_det_grouped = []
                list_mnorm = []
                list_vnorm = []
                for run in runlist:
                    list_raw.append(run + '_' + self._raw_ws)
                    list_monitor.append(run + '_' + self._monitor_ws)
                    list_det_grouped.append(run + '_' + self._det_grouped_ws)
                    list_mnorm.append(run + '_' + self._mnorm_ws)
                    list_vnorm.append(run + '_' + self._vnorm_ws)

                GroupWorkspaces(list_raw, OutputWorkspace=self._raw_ws)
                GroupWorkspaces(list_monitor, OutputWorkspace=self._monitor_ws)
                GroupWorkspaces(list_det_grouped, OutputWorkspace=self._det_grouped_ws)
                GroupWorkspaces(list_mnorm, OutputWorkspace=self._mnorm_ws)

                # set workspace properties
                self.setPropertyValue('RawWorkspace', self._raw_ws)
                self.setPropertyValue('MNormalisedWorkspace', self._mnorm_ws)
                self.setPropertyValue('DetGroupedWorkspace', self._det_grouped_ws)
                self.setPropertyValue('MonitorWorkspace', self._monitor_ws)

                if self._unmirror_option > 0 :
                    GroupWorkspaces(list_vnorm, OutputWorkspace=self._vnorm_ws)
                    self.setPropertyValue('VNormalisedWorkspace', self._vnorm_ws)
        else:
            # single run

            if self._save:
                self._save_reduced_ws(runlist[0] + '_' + self._red_ws )

            if self._plot:
                self._plot_reduced_ws(runlist[0])

            self.setPropertyValue('ReducedWorkspace', runlist[0] + '_' + self._red_ws)

            if self._control_mode:
                # Set properties
                self.setPropertyValue('RawWorkspace', runlist[0] + '_' + self._raw_ws)
                self.setPropertyValue('MNormalisedWorkspace', runlist[0] + '_' + self._mnorm_ws)
                self.setPropertyValue('DetGroupedWorkspace', runlist[0] + '_' + self._det_grouped_ws)
                self.setPropertyValue('MonitorWorkspace', runlist[0] + '_' + self._monitor_ws)
                if self._unmirror_option > 0 :
                    self.setPropertyValue('VNormalisedWorkspace', runlist[0] + '_' + self._vnorm_ws)
            else:
                # Cleanup unused workspaces
                DeleteWorkspace(runlist[0] + '_' + self._raw_ws)
                DeleteWorkspace(runlist[0] + '_' + self._mnorm_ws)
                DeleteWorkspace(runlist[0] + '_' + self._det_grouped_ws)
                DeleteWorkspace(runlist[0] + '_' + self._monitor_ws)

            if self._unmirror_option > 0 :
                    DeleteWorkspace(runlist[0] + '_' + self._vnorm_ws)



        print('Done')

    def _save_reduced_ws(self, ws):
        """
        Saves the reduced workspace
        """
        filename = mtd[ws].getName() + '.nxs'

        print('Save ' + filename + ' in current directory')

        #workdir = config['defaultsave.directory']
        #file_path = os.path.join(workdir, filename)
        SaveNexusProcessed(InputWorkspace=ws, Filename=filename)
        print('Saved file : ' + filename)

    def _plot_reduced_ws(self, run):
        """
        Plots the reduced workspace
        """
        _red = run + '_' + self._red_ws
        _summed_spectra = _red + '_toplot'

        print('Plot (all spectra summed)' + _summed_spectra)

        SumSpectra(InputWorkspace=_red, OutputWorkspace=_summed_spectra)

        from IndirectImport import import_mantidplot
        mtd_plot = import_mantidplot()
        graph = mtd_plot.newGraph()

        mtd_plot.plotSpectrum(_summed_spectra, 0, window=graph)

        layer = graph.activeLayer()
        layer.setAxisTitle(mtd_plot.Layer.Bottom, 'Energy Transfer (micro eV)')
        layer.setAxisTitle(mtd_plot.Layer.Left, '')
        layer.setTitle('')

        # Should not delete, since if the plot is open it disappears
        # DeleteWorkspace(_summed_spectra)

# Register algorithm with Mantid
AlgorithmFactory.subscribe(IndirectILLReduction)
