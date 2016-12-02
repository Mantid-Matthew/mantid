#include "MantidCurveFitting/Functions/CrystalFieldPeakUtils.h"

#include "MantidAPI/CompositeFunction.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/IPeakFunction.h"
#include "MantidCurveFitting/Constraints/BoundaryConstraint.h"

#include <algorithm>
#include <math.h>

namespace Mantid {
namespace CurveFitting {
namespace Functions {
namespace CrystalFieldUtils {

using namespace CurveFitting;
using namespace Kernel;
using namespace API;

/// Calculate the width of a peak cenrted at x using
/// an interpolated value of a function tabulated at xVec points
/// @param x :: Peak centre.
/// @param xVec :: x-values of a tabulated width function.
/// @param yVec :: y-values of a tabulated width function.
double calculateWidth(double x, const std::vector<double> &xVec,
                      const std::vector<double> &yVec) {
  assert(xVec.size() == yVec.size());
  auto upperIt = std::lower_bound(xVec.begin(), xVec.end(), x);
  if (upperIt == xVec.end() || x < xVec.front()) {
    return -1.0;
  }
  if (upperIt == xVec.begin()) {
    return yVec.front();
  }
  double lowerX = *(upperIt - 1);
  double upperX = *upperIt;
  auto i = std::distance(xVec.begin(), upperIt) - 1;
  return yVec[i] + (yVec[i + 1] - yVec[i]) / (upperX - lowerX) * (x - lowerX);
}

/// Set a boundary constraint on the appropriate parameter of the peak.
/// @param peak :: A peak function.
/// @param fwhm :: A width of the peak.
/// @param fwhmVariation :: A value by which the with can vary on both sides.
void setWidthConstraint(API::IPeakFunction &peak, double fwhm,
                        double fwhmVariation) {
  double upperBound = fwhm + fwhmVariation;
  double lowerBound = fwhm - fwhmVariation;
  bool fix = lowerBound == upperBound;
  if (!fix) {
    if (lowerBound < 0.0) {
      lowerBound = 0.0;
    }
    if (lowerBound >= upperBound) {
      lowerBound = upperBound / 2;
    }
  }
  if (peak.name() == "Lorentzian") {
    if (fix) {
      peak.fixParameter("FWHM");
      return;
    }
    peak.removeConstraint("FWHM");
    auto constraint = new Constraints::BoundaryConstraint(
        &peak, "FWHM", lowerBound, upperBound);
    peak.addConstraint(constraint);
  } else if (peak.name() == "Gaussian") {
    if (fix) {
      peak.fixParameter("Sigma");
      return;
    }
    const double WIDTH_TO_SIGMA = 2.0 * sqrt(2.0 * M_LN2);
    lowerBound /= WIDTH_TO_SIGMA;
    upperBound /= WIDTH_TO_SIGMA;
    peak.removeConstraint("Sigma");
    auto constraint = new Constraints::BoundaryConstraint(
        &peak, "Sigma", lowerBound, upperBound);
    peak.addConstraint(constraint);
  } else {
    throw std::runtime_error("Cannot set constraint on width of " +
                             peak.name());
  }
}

/// Calculate the number of visible peaks.
size_t calculateNPeaks(const API::FunctionValues &centresAndIntensities) {
  return centresAndIntensities.size() / 2;
}

/// Calculate the maximum number of peaks a spectrum can have.
size_t calculateMaxNPeaks(size_t nPeaks) { return nPeaks + nPeaks / 2 + 1; }

/// Set peak's properties such that it was invisible in the spectrum.
/// @param peak :: A peak function to set.
/// @param fwhm :: A width value to pass to the peak.
inline void ignorePeak(API::IPeakFunction &peak, double fwhm) {
  peak.setHeight(0.0);
  peak.fixAll();
  peak.setFwhm(fwhm);
}

/// Populates a spectrum with peaks of type given by peakShape argument.
/// @param spectrum :: A composite function that is a collection of peaks.
/// @param peakShape :: A shape of each peak as a name of an IPeakFunction.
/// @param centresAndIntensities :: A FunctionValues object containing centres
///        and intensities for the peaks. First nPeaks calculated values are the
///        centres and the following nPeaks values are the intensities.
/// @param xVec :: x-values of a tabulated width function.
/// @param yVec :: y-values of a tabulated width function.
/// @param fwhmVariation :: A variation in the peak width allowed in a fit.
/// @param defaultFWHM :: A default value for the FWHM to use if xVec and yVec
///        are empty.
/// @param nRequiredPeaks :: A number of peaks required to be created.
/// @param fixAllPeaks :: If true fix all peak parameters
/// @return :: The number of peaks that will be actually fitted.
size_t buildSpectrumFunction(API::CompositeFunction &spectrum,
                             const std::string &peakShape,
                             const API::FunctionValues &centresAndIntensities,
                             const std::vector<double> &xVec,
                             const std::vector<double> &yVec,
                             double fwhmVariation, double defaultFWHM,
                             size_t nRequiredPeaks, bool fixAllPeaks) {
  if (xVec.size() != yVec.size()) {
    throw std::runtime_error("WidthX and WidthY must have the same size.");
  }

  bool useDefaultFWHM = xVec.empty();
  auto nPeaks = calculateNPeaks(centresAndIntensities);
  auto maxNPeaks = calculateMaxNPeaks(nPeaks);
  if (nRequiredPeaks > maxNPeaks) {
    maxNPeaks = nRequiredPeaks;
  }
  for (size_t i = 0; i < maxNPeaks; ++i) {
    auto fun = API::FunctionFactory::Instance().createFunction(peakShape);
    auto peak = boost::dynamic_pointer_cast<API::IPeakFunction>(fun);
    if (!peak) {
      throw std::runtime_error("A peak function is expected.");
    }
    if (i < nPeaks) {
      auto centre = centresAndIntensities.getCalculated(i);
      peak->setCentre(centre);
      peak->setIntensity(centresAndIntensities.getCalculated(i + nPeaks));
      if (useDefaultFWHM) {
        peak->setFwhm(defaultFWHM);
      } else {
        auto fwhm = calculateWidth(centre, xVec, yVec);
        if (fwhm > 0.0) {
          peak->setFwhm(fwhm);
          setWidthConstraint(*peak, fwhm, fwhmVariation);
        } else {
          ignorePeak(*peak, defaultFWHM);
        }
      }
      peak->fixCentre();
      peak->fixIntensity();
    } else {
      ignorePeak(*peak, defaultFWHM);
    }
    if (fixAllPeaks) {
      peak->fixAll();
    }
    spectrum.addFunction(peak);
  }
  return nPeaks;
}

/// Update the peaks parameters after recalculationof the crystal field.
/// @param spectrum :: A composite function containings the peaks to update.
///                    May contain other functions (background) fix indices
///                    < iFirst.
/// @param centresAndIntensities :: A FunctionValues object containing centres
///        and intensities for the peaks. First nPeaks calculated values are the
///        centres and the following nPeaks values are the intensities.
/// @param nOriginalPeaks :: Number of actual peaks the spectrum had before the
///        update.This update can change the number of actual peaks.
/// @param iFirst :: The first index in the composite function (spectrum) at
///        which the peaks begin.
/// @param xVec :: x-values of a tabulated width function.
/// @param yVec :: y-values of a tabulated width function.
/// @param fwhmVariation :: A variation in the peak width allowed in a fit.
/// @return :: The new number of fitted peaks.
size_t updateSpectrumFunction(API::CompositeFunction &spectrum,
                              const FunctionValues &centresAndIntensities,
                              size_t nOriginalPeaks, size_t iFirst,
                              const std::vector<double> &xVec,
                              const std::vector<double> &yVec,
                              double fwhmVariation) {
  size_t nGoodPeaks = calculateNPeaks(centresAndIntensities);
  size_t maxNPeaks = spectrum.nFunctions() - iFirst;
  bool mustUpdateWidth = !xVec.empty();

  for (size_t i = 0; i < maxNPeaks; ++i) {
    auto fun = spectrum.getFunction(i + iFirst);
    auto &peak = dynamic_cast<API::IPeakFunction &>(*fun);
    if (i < nGoodPeaks) {
      auto centre = centresAndIntensities.getCalculated(i);
      peak.setCentre(centre);
      peak.setIntensity(centresAndIntensities.getCalculated(i + nGoodPeaks));
      if (mustUpdateWidth) {
        auto fwhm = peak.fwhm();
        auto expectedFwhm = calculateWidth(centre, xVec, yVec);
        if (expectedFwhm <= 0.0) {
          ignorePeak(peak, fwhm);
        } else if (fabs(fwhm - expectedFwhm) > fwhmVariation) {
          peak.setFwhm(expectedFwhm);
          setWidthConstraint(peak, expectedFwhm, fwhmVariation);
        }
      }
      peak.unfixIntensity();
      peak.fixIntensity();
    } else {
      peak.setHeight(0.0);
      if (i > nOriginalPeaks) {
        peak.fixAll();
      }
    }
  }
  return nGoodPeaks;
}

} // namespace CrystalFieldUtils
} // namespace Functions
} // namespace CurveFitting
} // namespace Mantid
