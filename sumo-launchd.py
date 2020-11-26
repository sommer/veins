#!/usr/bin/env python3

#
# Copyright (C) 2020 Christoph Sommer <sommer@cms-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import os
import sys

try:
    sys.stderr.write('WARNING: the sumo-launchd.py script is deprecated in favor of bin/veins_launchd. Redirecting.\n')
    sys.stderr.flush()
    sys.stdout.write('WARNING: the sumo-launchd.py script is deprecated in favor of bin/veins_launchd. Redirecting.\n')
    sys.stdout.flush()
    exec(open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'bin/veins_launchd')).read())
finally:
    sys.stderr.write('WARNING: the sumo-launchd.py script is deprecated in favor of bin/veins_launchd. Redirection done.\n')
    sys.stderr.flush()
    sys.stdout.write('WARNING: the sumo-launchd.py script is deprecated in favor of bin/veins_launchd. Redirection done.\n')
    sys.stdout.flush()

