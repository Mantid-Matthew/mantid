#  This file is part of the mantid workbench.
#
#  Copyright (C) 2017 mantidproject
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""Provides a widget to wrap common behaviour for all plugins"""
from __future__ import absolute_import

from qtpy.QtCore import Qt
from qtpy.QtWidgets import QDockWidget, QWidget


class PluginWidget(QWidget):

    ALLOWED_AREAS = Qt.AllDockWidgetAreas
    LOCATION = Qt.LeftDockWidgetArea
    FEATURES = QDockWidget.DockWidgetClosable | QDockWidget.DockWidgetFloatable

    def __init__(self, main_window):
        QWidget.__init__(self, main_window)

        self.main = main_window

# ----------------- Plugin API --------------------

    def register_plugin(self):
        raise NotImplementedError()

    def get_plugin_title(self):
        raise NotImplementedError()

# ----------------- Plugin behaviour ------------------

    def create_dockwidget(self):
        """Creates a QDockWidget suitable for wrapping
        this plugin"""
        dock = QDockWidget(self.get_plugin_title(), self.main)
        dock.setObjectName(self.__class__.__name__+"_dockwidget")
        dock.setAllowedAreas(self.ALLOWED_AREAS)
        dock.setFeatures(self.FEATURES)
        dock.setWidget(self)
        return dock, self.LOCATION
