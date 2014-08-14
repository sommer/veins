import os
import sys
from subprocess import check_call
from logging import warning, error
from optparse import OptionParser
import fnmatch


class VeinsConfigure(object):

    """
    Configure class for a basic Veins setup
    """

    def __init__(self):
        self.run_libs = [os.path.join('src', 'veins')]
        self.run_neds = ['src']
        self.options = None

    def option_parser(self):
        """ create option parser for handling Veins options """
        parser = OptionParser()
        return parser

    def parse_args(self):
        (self.options, args) = self.option_parser().parse_args()
        if args:
            warning("Superfluous command line arguments: \"%s\"" % " ".join(args))

    def output_directory(self):
        """ get output directory """
        return 'out'

    def opp_makemake(self):
        """ generate OMNeT++ makefile """
        check_call(['env', 'opp_makemake'] + self.opp_makemake_flags(), cwd='src')

    def opp_makemake_flags(self):
        """ build array with opp_makemake flags """
        flags = ['-f', '--deep', '--make-so', '-o', 'veins', '-O', self.output_directory()]
        return flags


class InetConfigure(VeinsConfigure):

    """
    Configure class for Veins with INET framework
    """

    def option_parser(self):
        parser = super(InetConfigure, self).option_parser()
        parser.add_option("--with-inet", dest="inet", help="link Veins with a version of the INET Framework installed in PATH [default: do not link with INET]", metavar="PATH")
        return parser

    def opp_makemake_flags(self):
        flags = super(InetConfigure, self).opp_makemake_flags()
        if self.options.inet:
            inet_root = self.options.inet
            InetConfigure.check_version(inet_root)

            inet_header_dirs = set()
            inet_src_path = os.path.join(inet_root, 'src')
            for root, dirnames, filenames in os.walk(inet_src_path):
                for filename in fnmatch.filter(filenames, '*.h'):
                    inet_header_dirs.add(os.path.relpath(os.path.dirname(os.path.join(root, filename)), 'src'))
            inet_includes = ['-I' + s for s in inet_header_dirs]
            inet_link = ["-L" + os.path.join(os.path.relpath(inet_root, 'src'), 'src'), "-linet"]
            inet_defs = ["-DINET_IMPORT", "-DWITH_INET"]

            flags += inet_includes + inet_link + inet_defs
            self.run_libs.insert(0, os.path.relpath(os.path.join(inet_root, 'src', 'inet')))
            self.run_neds.insert(0, os.path.relpath(os.path.join(inet_root, 'src')))
        else:
            flags += ['-X' + os.path.join('src', 'inet')]

        return flags

    @staticmethod
    def check_version(inet_root):
        fname = os.path.join(inet_root, 'Version')
        try:
            with open(fname, 'r') as file:
                version = file.read().rstrip()
                if not version == 'inet-2.3.0':
                    warning('Unsupported INET Version. Expecting inet-2.3.0, found "%s"' % version)
        except IOError as e:
            error('Could not determine INET Version: %s' % e)
            sys.exit(1)


def create_output_directory(config):
    path = os.path.realpath(config.output_directory())
    if not os.path.isdir(path):
        os.makedirs(path)


def write_config(config):
    create_output_directory(config)
    f = open(os.path.join(config.output_directory(), 'config.py'), 'w')
    f.write('run_libs = %s\n' % repr(config.run_libs))
    f.write('run_neds = %s\n' % repr(config.run_neds))
    f.close()


def run_configure(config_class):
    configure = config_class()
    configure.parse_args()

    # create files
    write_config(configure)
    configure.opp_makemake()
