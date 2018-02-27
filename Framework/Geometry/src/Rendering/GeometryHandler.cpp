#include "MantidGeometry/Rendering/GeometryHandler.h"
#include "MantidGeometry/Instrument/RectangularDetector.h"
#include "MantidGeometry/Objects/CSGObject.h"
#include "MantidGeometry/Rendering/GeometryTriangulator.h"
#include "MantidGeometry/Rendering/Renderer.h"
#include <boost/make_shared.hpp>

namespace Mantid {
namespace Geometry {
GeometryHandler::GeometryHandler(IObjComponent *comp)
    : m_renderer(new detail::Renderer()), m_objComp(comp) {}

GeometryHandler::GeometryHandler(boost::shared_ptr<CSGObject> obj)
    : m_renderer(new detail::Renderer()),
      m_triangulator(new detail::GeometryTriangulator(obj.get())),
      m_obj(obj.get()) {}

GeometryHandler::GeometryHandler(CSGObject *obj)
    : m_renderer(new detail::Renderer()),
      m_triangulator(new detail::GeometryTriangulator(obj)), m_obj(obj) {}

GeometryHandler::GeometryHandler(const GeometryHandler &handler)
    : m_renderer(new detail::Renderer()) {
  if (handler.m_obj) {
    m_obj = handler.m_obj;
    if (handler.m_triangulator)
      m_triangulator.reset(new detail::GeometryTriangulator(m_obj));
  }
  if (handler.m_objComp)
    m_objComp = handler.m_objComp;
  if (handler.m_shapeInfo)
    m_shapeInfo = handler.m_shapeInfo;
}

boost::shared_ptr<GeometryHandler> GeometryHandler::clone() const {
  return boost::make_shared<GeometryHandler>(*this);
}

GeometryHandler::~GeometryHandler() {}

void GeometryHandler::render() const {
  if (m_shapeInfo)
    m_renderer->renderShape(*m_shapeInfo);
  else if (m_objComp != nullptr)
    m_renderer->renderIObjComponent(*m_objComp);
  else if (canTriangulate())
    m_renderer->renderTriangulated(*m_triangulator);
}

void GeometryHandler::initialize() const {
  if (m_obj != nullptr)
    m_obj->updateGeometryHandler();
  render();
}

size_t GeometryHandler::numberOfTriangles() const {
  if (canTriangulate())
    return m_triangulator->numTriangleFaces();
  return 0;
}

size_t GeometryHandler::numberOfPoints() const {
  if (canTriangulate())
    return m_triangulator->numTriangleVertices();
  return 0;
}

const std::vector<double> &GeometryHandler::getTriangleVertices() const {
  static std::vector<double> empty;
  if (canTriangulate())
    return m_triangulator->getTriangleVertices();
  return empty;
}

const std::vector<int> &GeometryHandler::getTriangleFaces() const {
  static std::vector<int> empty;
  if (canTriangulate())
    return m_triangulator->getTriangleFaces();
  return empty;
}

void GeometryHandler::setGeometryCache(size_t nPts, size_t nFaces,
                                       std::vector<double> &&pts,
                                       std::vector<int> &&faces) {
  if (canTriangulate()) {
    m_triangulator->setGeometryCache(nPts, nFaces, std::move(pts),
                                     std::move(faces));
  }
}

void GeometryHandler::GetObjectGeom(detail::ShapeInfo::GeometryShape &mytype,
                                    std::vector<Kernel::V3D> &vectors,
                                    double &myradius, double &myheight) const {
  mytype = detail::ShapeInfo::GeometryShape::NOSHAPE;
  if (m_shapeInfo)
    m_shapeInfo->getObjectGeometry(mytype, vectors, myradius, myheight);
}

void GeometryHandler::setShapeInfo(detail::ShapeInfo &&shapeInfo) {
  m_triangulator.reset(nullptr);
  m_shapeInfo.reset(new detail::ShapeInfo(std::move(shapeInfo)));
}
} // namespace Geometry
} // namespace Mantid
