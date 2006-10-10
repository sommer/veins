#include <omnetpp.h>
#include "channelStateMsg_m.h"

// module class declaration:
class channelStateCalc:public cSimpleModule {
      public:
    Module_Class_Members(channelStateCalc, cSimpleModule, 0)
    virtual void initialize();
    virtual void handleMessage(cMessage * msg);
    virtual void finish();

      private:
    double calculatePathLoss(double distance);
    double calculateShadowing();
    double calculateFading(int sb, double frequency, double mobile_speed);
    void initFading();

    int LIGHTSPEED;

    // for path loss
    double TENLOGK;
    double ALPHA;

    // for shading loss
    double MEAN;
    double STD_DEV;

    // for fading loss
    double DELAY_RMS;               // Mean Delay Spread
    double FREQUENCY_SPACING;       // frequency sample spacing
    int FADING_PATHS;               // number of different simulated fading paths
    int CENTER_FREQUENCY;           // center frequency
    int SUBBANDS;

    // describtion of the different fading paths
    double **angle_of_arrival;
    double **delay;

    int CALCULATE_PATH_LOSS;
    int CALCULATE_SHADOWING;
    int CALCULATE_FADING;

    int CORRELATED_SUBBANDS;
    int USE_LINEAR_SCALE;
};

// module type registration:
Define_Module(channelStateCalc);

void channelStateCalc::initialize()
{
    ev << "channelStateCalc->initialize()" << endl;

    LIGHTSPEED = par("lightspeed");
    TENLOGK = par("tenlogk");
    ALPHA = par("alpha");

    MEAN = par("mean");
    STD_DEV = par("std_dev");   // 250 = 1 DL, 250*500=125000 = 1 second

    FADING_PATHS = par("fadingPaths");
    CENTER_FREQUENCY = par("center_frequency");
    DELAY_RMS = par("delay_rms");
    FREQUENCY_SPACING = par("freq_spacing");
    SUBBANDS = par("subbands");
    initFading();

    CALCULATE_PATH_LOSS = par("calculatePathLoss");
    CALCULATE_SHADOWING = par("calculateShadowing");
    CALCULATE_FADING = par("calculateFading");

    CORRELATED_SUBBANDS = par("correlated_subbands");
    USE_LINEAR_SCALE = par("use_linear_scale");
}

// implementation of the module class:
void channelStateCalc::handleMessage(cMessage * msg)
{
    channelStateMsg *channelStatemsg = (channelStateMsg *) msg;

    double speed = channelStatemsg->getSpeed();
    double distance = channelStatemsg->getDistance();
    double pathLoss = 0;
    double shadowLoss = 0;
    double fadingLoss = 0;

    if(CALCULATE_PATH_LOSS) {
        pathLoss = calculatePathLoss(distance);
    }
    if(CALCULATE_SHADOWING){
        shadowLoss = calculateShadowing();
    }
    for (int sb = 0; sb < SUBBANDS; sb++) { //over subbands
        if(CALCULATE_FADING) {
            // calculate center frequency of each sub band
            double frequency = CENTER_FREQUENCY - SUBBANDS * FREQUENCY_SPACING / 2 + sb * FREQUENCY_SPACING + FREQUENCY_SPACING / 2;    // works correct for odd and even subbands
            fadingLoss = calculateFading(sb, frequency, speed); // calculate frequency dependend fading for each subband
        }
        double loss = 0 + pathLoss + shadowLoss + fadingLoss;
        if (USE_LINEAR_SCALE) loss = pow(10, (loss/10));    // convert to linear scale if selected
        channelStatemsg->setChannelState(sb, loss);
    }
    send(channelStatemsg, "out");
}

void channelStateCalc::finish()
{
    for (int s = 0; s < SUBBANDS; s++)
        delete []angle_of_arrival[s];
    delete []angle_of_arrival;

    for (int s = 0; s < SUBBANDS; s++)
        delete []delay[s];
    delete []delay;

    ev << "channelStateCalc->finish()" << endl;
}

/* Determined by the distance and medium-specific parameters the
 * path loss from the terminal to the base station is calculated
 */
double channelStateCalc::calculatePathLoss(double distance)
{
    return TENLOGK - 10 * ALPHA * log10(distance);
}

// calculates shadowling loss based on a normal gaussian function
double channelStateCalc::calculateShadowing()
{
    return -1 * normal(MEAN, STD_DEV);
}

void channelStateCalc::initFading()
{
    angle_of_arrival = new double *[SUBBANDS];
    for (int s = 0; s < SUBBANDS; s++)
        angle_of_arrival[s]  = new double [FADING_PATHS];

    delay = new double *[SUBBANDS];
    for (int s = 0; s < SUBBANDS; s++)
        delay[s]  = new double [FADING_PATHS];

    for (int s = 0; s < SUBBANDS; s++) {
        for (int i = 0; i < FADING_PATHS; ++i) {
            //angle of arrival on path i, used for doppler_shift calculation
            //might be also subband independent, i.e. per s
            angle_of_arrival[s][i] = cos(uniform(0,M_PI));
            //delay on path i
            //might be also subband independent, i.e. per s
            delay[s][i] = (double)exponential(DELAY_RMS);
        }
    }
}

/*
 * Jakes-like  method, Frequency in Megahertz
 * With OFDM subbands (numbered from low to high f):
 * frequency = CENTER_FREQUENCY - SUBBANDS * FREQUENCY_SPACING / 2 + sb * FREQUENCY_SPACING + FREQUENCY_SPACING / 2
 */
double channelStateCalc::calculateFading(int sb, double frequency, double mobile_speed)
{
  double phi_d = 0;
  double phi_i = 0;
  double phi = 0;
  double phi_sum = 0;

  double re_h = 0;
  double im_h = 0;

  if (CORRELATED_SUBBANDS) sb = 0;  // sub bands are correleted -> same fading 'profile'

  double doppler_shift = mobile_speed * frequency / LIGHTSPEED ;
//  ev << " Maximum Doppler Frequency is: "<< doppler_shift << " at speed " << mobile_speed << endl;

  for(int i = 0; i < FADING_PATHS; i++)
  {
    //some math for complex numbers:
    //z = a + ib        cartesian form
    //z = p * e ^ i(phi)    polar form
    //a = p * cos(phi)
    //b = p * sin(phi)
    //z1 * z2 = p1 * p2 * e ^ i(phi1 + phi2)

    phi_d = angle_of_arrival[sb][i] * doppler_shift;    // phase shift due to doppler => t-selectivity
    phi_i = delay[sb][i] * frequency;                   // phase shift due to delay spread => f-selectivity

    phi = 2.00 * M_PI * (phi_d * simTime() - phi_i);    // calculate resulting phase due to t-and f-selective fading

    //one ring model/Clarke's model plus f-selectivity according to Cavers:
    //due to isotropic antenna gain pattern on all paths only a^2 can be received on all paths
    //since we are interested in attenuation a:=1, attenuation per path is then:
    double attenuation = (1.00/sqrt(FADING_PATHS));

    //convert to cartesian form and aggregate {Re, Im} over all fading paths
    re_h = re_h + attenuation * cos(phi);
    im_h = im_h - attenuation * sin(phi);
  }
  //output: |H_f|^2= absolute channel impulse response due to fading in dB
  //note that this may be >0dB due to constructive interference
  return 10 * log10(re_h * re_h + im_h * im_h);
}

/*
  {
    phi_sum = phi_sum + phi;    // sum over all fading paths
  }
  double gain_re = 1.00/sqrt(FADING_PATHS) * cos(phi_sum); // convert from polar to cartesian form because we want the absolute value
  double gain_im = 1.00/sqrt(FADING_PATHS) * sin(phi_sum); // convert from polar to cartesian form because we want the absolute value
  double gain = (10 * log10(gain_re * gain_re + gain_im * gain_im));    //output attenuation^2 (power) in dB
  return gain;
}
*/
