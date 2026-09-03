Veins - The open source vehicular network simulation framework.

See the Veins website <http://veins.car2x.org/> for a tutorial, documentation,
and publications.

Veins is composed of many parts. See the version control log for a full list of
contributors and modifications. Each part is protected by its own, individual
copyright(s), but can be redistributed and/or modified under an open source
license. License terms are available at the top of each file. Parts that do not
explicitly include license text shall be assumed to be governed by the "GNU
General Public License" as published by the Free Software Foundation -- either
version 2 of the License, or (at your option) any later version
(SPDX-License-Identifier: GPL-2.0-or-later). Parts that are not source code and
do not include license text shall be assumed to allow the Creative Commons
"Attribution-ShareAlike 4.0 International License" as an additional option
(SPDX-License-Identifier: GPL-2.0-or-later OR CC-BY-SA-4.0). Full license texts
are available with the source distribution.

=================================================================
Vanetza Integration
=================================================================

This project extends Veins by integrating the Vanetza V2X protocol stack.
The repository does not include the Vanetza source code or binaries.
Vanetza must be installed separately before building this project.

Prerequisites
-------------

1. Clone the Vanetza repository into your home directory:

   cd ~
   git clone https://github.com/riebl/vanetza.git vanetza

2. Build Vanetza by following the official build instructions provided by the
   Vanetza project.

3. Verify that the following directories exist:

   ~/vanetza
   ├── build/
   │   └── lib/
   │       └── static/
   └── vanetza/

4. Build this project normally:

   make

Build Configuration
-------------------

The build system is configured to automatically search for Vanetza in:

   ~/vanetza

using the HOME environment variable.

If Vanetza is installed in this location, no additional configuration is
required.

If Vanetza is missing or installed in a different location without updating the
build configuration, compilation will fail because the required header files
and libraries cannot be found.
