//
//@titlepage
//<h1>MiXiM API Reference</h1>
//
//On the left side you can browse the ned-files documentation.
//
//Under the topic "C++" you will find more detailed information on the C++ classes of 
//MiXiM which is a good start for browsing the documentation.
//
//More Details regarding the MoBAN modules can be found under "Selected Topics"
//or at the <a href="moban.html"> MoBAN posture specification file</a>
//<hr>
//
//More documentation such as installation instructions, tutorials, how-tos, 
//and an overview on the examples can be found at the MiXiM wiki at the <a href="http://apps.sourceforge.net/trac/mixim/" target="_blanc">Sourceforge Project Wiki</a>
//
//
// @page moban.html, MoBAN Implementation Reference
//<h1>MoBAN Implementation Reference</h1>
//
// MoBAN is a configurable mobility model for Wireless Body Area Networks (WBANs). This is the implementation of MoBAN on top of MiXiM framework. 
// The model has been specifically designed so that it can be configured for being used for performance evaluation of a broad range of application 
// scenarios including WBANs. Both global movement of the WBAN and the individual node mobility within the WBAN have been taken into consideration. 
// The model can be used in simulating both intra- and extra-WBAN protocols. The Model is constructed based on the RPGM group mobility model. 
// It implements various time and space correlations for selecting the posture of the body and the movement behavior. The structural block diagram 
// of the MoBAN implementation is shown in this figure:
//
//
// <img src="../images/OmnetIm.png" border = "20" width="570" height="350" alt="MoBAN implementation block diagram." />
//
// 
// 
// The model implementation consists of two NED modules:
// <li> <b> MoBANCoordinator </b> </li> 
// <li> <b> MoBANLocal </b> </li>
// 
// MoBANCoordinator module is the module that coordinates the whole WBAN movement. It decides about the location of the WBAN in the simulation area, 
// the movement behavior, and the WBAN posture at any time. One MoBANCoordinator module should be instantiated for each WBAN that exist in an application. 
// Thos module has two xml parameters which should be assigned to two xml files, a postures definition file and a configuration file. These file can be 
// same for all coordinators in the network. In this case, all WBANs are supposed to have the same posture specifications and statistically same mobility 
// behavior. However, different files can be given as well. More information about input XML files can be found in the following links.
//
// <li>  <a href="xmlposturespec.html"> MoBAN posture specification file</a> </li> 
// <li>  <a href="xmlconfigfile.html"> MoBAN configuration file</a> </li> 
// 
// Any WBAN node in the network must use MoBANLocal as its mobility module in the definition of the node. MoBANLocal receives its reference point as well 
// as radius and the speed of its local movement behavior from the MoBANCoordinator module. Actually, this module is responsible to determine the position 
// of the node in the simulation area by combining the relative local movement and the given reference point for global movement. 
//
// MoBANLocal module has a parameter named "coordinatorIndex" that determines the index of the belonging coordinator module. A coordinator module looks 
// for all modules in the network with name "node" which their mobility submodule has this parameter. Then if the value of this parameter is the same as 
// the index of that coordinator, the node is assumed to belong to that coordinator. 
//
//     
//  
// On the left side you can browse the ned-files documentation.
//
// Under the topic "C++" you will find more detailed information on the C++ classes of 
// MoBAN which is a good start for browsing the documentation.
//
//
// <hr>
//
// <b>More information about the MoBAN mobility Model can be found <a href="http://www.es.ele.tue.nl/nes/" target="_blanc"> here </a></b>.
//
// <b>To import this project and run existing examples, please visit <a href="projectdoc.html"> this page </a></b>.
//
// <b>Information about using the MoBAN model in a simulation network can be found in <a href="usedoc.html"> this page </a></b>.
//
// <hr>
// @author <a href="http://www.es.ele.tue.nl/~mnabi" target="_blanc"> Majid Nabi </a>
// @date February, 2011
// @version V1.0 
//
// Electronic Systems group (ES), Eindhoven University of Technology (TU/e), the Netherlands.
//
// @page projectdoc.html, Project and Examples
//
// This project contains the implementation of the MoBAN mobility model as well as examples of exploiting MoBAN for simulating wireless networks
// including WBANs. The <i>modules</i> directory, includes NED files of the MOBAN modules and their c++ description files. The <i>examples</i> directory 
// provides examples of using the model. 
//
// To run the examples, the project should be imported to the framework in OMNeT++ IDE. The MiXiM is the reference project for MoBAN 
// project. So MiXiM should already be installed in the framework. To import the MoBAN project, follow the following in the IDE.
//
// File -> Import... -> General (Existing Project into Workspace)
//
// Here you can either select the root directory of the MoBAN project or select the archive file. By finishing this step, the MoBAN project is imported 
// to the current workspace, next to the MiXiM project.
//
// The BaseMoBANNetwork is the main network of the examples which uses the base network protocol stack existing in the MiXiM and includes the desired number of MoBANCoordinator 
// modules. BaseNode module which defines a wireless node is imported from the baseNetwork examples of the MiXiM. All submodules of BaseNode (including the mobility module type)
// can be set through the '.ini' configuration file. In the case that a node is a WBAN node, the mobility type should be MoBANLocal. Two examples is provided which can be 
// selected by including either 'example1.ini' or 'example2.ini' in the top of 'omnetpp.ini' file.
//
// Note that MoBAN itself does not make any assumption about the position of nodes on the body and any other thing which
// might be related to the specific application scenario. The definition of all possible postures in the target application and their complete 
// specifications are fed to the model by an XML file. The file path is specified by the 'postureSpecFile' parameter of the 'MoBANCoordinator' module. For more 
// detail about making the posture specification xml file, see <a href="xmlposturespec.html">XML file for posture specification</a>. Moreover, a configuration file
// determines the initial status and the spatial and temporal correlations in selecting the posture. It is given to the model by specifying an xml file 
// to the 'configFile' parameter of the 'MoBANCoordinator' module. For more detail about the structure of this xml file, visit  
// <a href="xmlconfigfile.html"> XML configuration file</a>. In both examples, the node order and positions are set according to the following figure. 
//
// <img src="../images/WBAN.png" border = "20" width="200"  alt="Position of nodes in the examples." />
//
// The first example is a single WBAN consisting of 12 wireless nodes. To have a better view in the graphical display, All dimensions are supposed 
// to be in centimeters in this example. Obviously, the physical layer should take it into account. This is actually an example of using MoBAN for
// simulating an intra-WBAN protocol without considering the surrounding network. Moreover, in this example, the graphical display is from front view. 
// It means we see x and z axis in the display. These are set in the 'postures1.xml' file, to make the behavior of the posture changes visible (just for demonstration purpose).
// No space-time correlation is defined in the 'configMoBAN1.xml'. Thus, the posture selection is uniform random.
//
// The second example shows how MoBAN can be used for simulation of a full network including WBANs and a surrrounding network.
// The example is a network consisting of 44 sensor nodes from which 20 nodes are supposed to form an ambient (static) network. The remaining 24 nodes,
// are WBAN nodes of two different WBAN. So every WBAN is supposed to have 12 sensor nodes. Therefore, two MoBANCorrdinator module is instantiated in the
// network simulation. The dimensions specified in this example are in meters. So the WBAN movement behavior might not be well visible in the graphical display.
// However, it does not have anything to do with the accuracy of the simulation. It is just an issue in the graphical display, because OMNeT IDE considers bigger 
// dimension. 
//     
// In this example, the spatial and temporal correlation is exploited. Two area types ('BedRoom','LivingRoom') and two time domains ('Day','Night') are defined 
// 'configMoBAN.xml' file. A Markov transition matrix is given which is considered as the base Markov transition matrix. Three steady state vectors are also specified.
// So for each space-time combination, a particular transition matrix will be used, either the base matrix or a derived matrix from the given steady state vector.  
//
// <b>NOTE:</b> Although the examples are developed to demonstrate the use of MoBAN as the mobility model, the provided posture specification can be a 
// reprehensive for typical postures of a human body. So it might be used in your simulation. Of course, it is possible to adapt the posture file to your 
// application scenario. For instance, if you are simulating an indoor network, you may want to exclude 'RUNNING' posture. 
//
//@page usedoc.html, Using MoBAN in your Simulations
// This is a short guide for exploiting the MoBAN mobility model in your simulation, if there are some kind of WBAN, or group mobility in your application. Note that
// in the simplest case, MoBAN implements RPGM mobility model, if just one posture is defined in the input posture specification file. However, the appropriate setting is 
// chosen according to the requirements.
// 
// You can use the model either with importing the project into your framework or by importing source files to your project. If your simulation is not done inside 
// the MiXiM project, and you have a separate project for which the MiXiM project is set as a reference project, then you can simply import the MoBAN project and 
// set it as another reference for your project. Now, you can use the MoBAN mobility model inside your project by adding the following line on top of your NED module definitions.
//
// import moban.mobility.*
//
// In the case that you are doing simulation within the MiXiM project, it is not possible to use the above method, as recursive reference projects are not allowed.
// Here, you may copy all files in the 'moban/modules/mobility' directory to the 'MiXiM/modules/mobility' directory of the MiXiM project. Then you need to make a 
// change in the NED files as follows:
//
// In MoBANCoordinator.ned and MoBANLocal.ned,       <tt> package moban.mobility; </tt>  ----> <tt> package org.mixim.modules.mobility;</tt>   
//
// Doing so, the MoBAN mobility model can be used in your simulation model like as any other mobility model existing in the mobility framework in MiXiM.
//
//
//@page xmlposturespec.html, XML File for Posture Specification
// This XML file defines the possible postures for the MOBAN model by determining all specification of each posture. 
// The parameter that should be defined for each posture is the speed range of the global movement of the WBAN in the simulation area, and
// the parameters of all nodes in the WBAN. The relative position of a each node (3D coordination of the reference point), the speed of local (individual)
// movement of each node, and the radius of the sphere around each node that node can move inside are parameters that should be set individually for each posture.
// 
// Each posture can have a name that is used for showing in display during simulation, if the graphical display is used in simulation. Unique and subsequent ID numbers
// should also be given to the postures.
//
// An XML schema shows the acceptable format of input XML file for posture specification. <a href="../images/postures.xsd" target="_blanc"> <b>Download XML schema</b></a>.
// An example of xml file for posture specification is given <a href="../images/postures.xml" target="_blanc">here</a>.
//
//@page xmlconfigfile.html, XML File for MoBAN Configuration
// This XML file provides the configuration options for the MOBAN model. The parameters that might be set in this file are as follows.
// <li> <b>Initial posture of the WBAN</b></li> optional
//  
// <li> <b>Initial position of the WBAN in the simulation area</b></li>optional
//
// <li> <b>Duration range for the postures</b></li>(optional) A uniform random value is selected whenever a new stable posture is selected. If lower bound is equal to the upper
// bound, the constant duration is always used. Note that the various length of different postures is implied in the Markov model which might be specified. This duration
// determines the length of one state in the Markov Model.
// 
// <li> <b>A list of Markov transition matrices</b></li> Any number of Markov transition matrices for posture selection can be separately defined. If no matrix is given,
// a uniform random strategy is used for posture selection. If posture selection is done in the same way with no spatial or temporal correlation, just one matrix is 
// enough. 
//
// In the case that spatial or time correlation is required, several matrices can be defined among which one should have the Attribute 'type' with value "default". 
// The default matrix is firstly used for the positions or time instances that no specific matrix is set. Moreover, there is the possibility that a steady state is defined 
// instead of specifying the whole Markov transition matrix. In this case, MoBAN extract the closest Markov model to the default matrix which satisfies
// the given steady vector. This is considered as a simpler way of assigning different posture patterns for different time and space domains.
//
// Note that each markovMatrix in the input xml file should be either a square matrix with dimensions equal to the number of postures, or a vector with the 
// number of elements equal to the number of postures (steady state vector). It is clear that the default matrix cannot be a steady state vector and should be
// a square matrix.  
// 
// A unique name should be given to each matrix, which will be used for specifying the matrix for particular time or area domain.
//
// <li> <b>A list of area types</b></li>. It is necessary, if spatial correlation is required for posture selection. If so, any number of area type can be defined each with a unique name.
// For each area type, a set of area boundaries can be set that determine the part of the simulation area that is of that area type. At the moment, just rectangular parts are supported.
//
// <li> <b>A list of time domains</b></li>. It is necessary, if temporal correlation is required for posture selection. If so, any number of time domains can be defined each with a unique name.
// For each time domain, a set of time boundaries can be set that determine the time durations of the whole simulation run of that time domain. 
//
// <li> <b>A list of space nad/or time combinations</b></li>. Using this tag in the XML file, we specify which matrix should be used for 
// each combination of the space and/or time. If in a combination, no time is specified, it means that it is the case for the whole simulation time. The
//
// An XML schema shows the acceptable format of input XML file for posture specification. <a href="../images/configureMoBAN.xsd" target="_blanc"> <b>Download XML schema</b></a>.
// An example of xml file for posture specification is given <a href="../images/configureMoBAN.xml" target="_blanc">here</a>.
//  

package org.mixim.base;

