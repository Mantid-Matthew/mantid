#ifndef MANTID_GEOMETRY_RENDERER_H_
#define MANTID_GEOMETRY_RENDERER_H_

#include "MantidGeometry/DllConfig.h"
#include "MantidGeometry/Rendering/OpenGL_Headers.h"
#include <vector>

class TopoDS_Shape;

namespace Mantid {
namespace Kernel {
class V3D;
}
namespace Geometry {
class RectangularDetector;
class StructuredDetector;
class IObjComponent;
class ComponentInfo;

namespace detail {
class GeometryTriangulator;
}

/** Renderer : Handles rendering details of geometry within mantid.

  Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
namespace detail {
class ShapeInfo;

class MANTID_GEOMETRY_DLL Renderer {
public:
  Renderer() = default;
  ~Renderer() = default;

  /// Render IObjComponent
  void renderIObjComponent(const IObjComponent &objComp) const;
  /// Render Traingulated Surface
  void renderTriangulated(detail::GeometryTriangulator &triangulator) const;
  /// Renders a sphere, cuboid, hexahedron, cone or cylinder
  void renderShape(const ShapeInfo &shapeInfo) const;

private:
  /// General method for rendering geometry
  template <typename... Args> void render(Args &&... args) const &;
  /// Renders a sphere
  void renderSphere(const ShapeInfo &shapeInfo) const;
  /// Renders a cuboid
  void renderCuboid(const ShapeInfo &shapeInfo) const;
  /// Renders a Hexahedron from the input values
  void renderHexahedron(const ShapeInfo &shapeInfo) const;
  /// Renders a Cone from the input values
  void renderCone(const ShapeInfo &shapeInfo) const;
  /// Renders a Cylinder/Segmented cylinder from the input values
  void renderCylinder(const ShapeInfo &shapeInfo) const;
  // general geometry
  /// Render IObjComponent
  void doRender(const IObjComponent &ObjComp) const;
  /// Render Traingulated Surface
  void doRender(GeometryTriangulator &triangulator) const;
#ifdef ENABLE_OPENCASCADE
  /// Render OpenCascade Shape
  void doRender(const TopoDS_Shape &ObjSurf) const;
#endif
};

template <typename... Args> void Renderer::render(Args &&... args) const & {
  // Wait for no OopenGL error
  while (glGetError() != GL_NO_ERROR)
    ;
  doRender(std::forward<Args>(args)...);
}

} // namespace detail
} // namespace Geometry
} // namespace Mantid

#endif /* MANTID_GEOMETRY_RENDERER_H_ */
