#include "MantidGeometry/MDGeometry/UnknownFrame.h"

namespace Mantid {
namespace Geometry {

UnknownFrame::UnknownFrame(std::unique_ptr<Kernel::MDUnit> unit)
    : m_unit(unit.release()) {}

UnknownFrame::UnknownFrame(const Kernel::UnitLabel &unit)
    : m_unit(new Mantid::Kernel::LabelUnit(unit)) {}

UnknownFrame::~UnknownFrame() {}

const std::string UnknownFrame::UnknownFrameName = "Unknown frame";

bool UnknownFrame::canConvertTo(const Mantid::Kernel::MDUnit &otherUnit) const {
  return false; // Cannot convert since it is unknown
}

std::string UnknownFrame::name() const { return UnknownFrameName; }

Mantid::Kernel::UnitLabel UnknownFrame::getUnitLabel() const {
  return m_unit->getUnitLabel();
}

const Mantid::Kernel::MDUnit &UnknownFrame::getMDUnit() const {
  return *m_unit;
}

Mantid::Kernel::SpecialCoordinateSystem
UnknownFrame::equivalientSpecialCoordinateSystem() const {
  return Mantid::Kernel::SpecialCoordinateSystem::None;
}

UnknownFrame *UnknownFrame::clone() const {
  return new UnknownFrame(std::unique_ptr<Kernel::MDUnit>(m_unit->clone()));
}
}
}
