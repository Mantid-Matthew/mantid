#include "MantidGeometry/IDetector.h"
#include <boost/python/class.hpp>
#include <boost/python/register_ptr_to_python.hpp>

using Mantid::Geometry::IDetector;
using Mantid::Geometry::IObjComponent;
using namespace boost::python;

void export_IDetector()
{
  register_ptr_to_python<boost::shared_ptr<IDetector> >();
  
  class_<IDetector, bases<IObjComponent>, boost::noncopyable>("IDetector", no_init)
    .def("getID", &IDetector::getID, "Returns the detector ID")
    .def("is_masked", &IDetector::isMasked, "Returns the value of the masked flag. True means ignore this detector")
    .def("is_monitor", &IDetector::isMonitor, "Returns True if the detector is marked as a monitor in the IDF")
    .def("solid_angle", &IDetector::solidAngle, "Return the solid angle in steradians between this detector and an observer")
    .def("get_twotheta", &IDetector::getTwoTheta, "Calculate the angle between this detector, another component and an axis")
    .def("get_phi", &IDetector::getPhi, "Returns the azimuthal angle of this detector")
    ;
}

