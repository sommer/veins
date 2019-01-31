#include "veins/base/phyLayer/PhyUtils.h"

#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

using namespace std;
using Veins::Radio;

Radio::Radio(int numRadioStates, bool recordStats, int initialState, int currentChannel, int nbChannels)
    : state(initialState)
    , nextState(initialState)
    , numRadioStates(numRadioStates)
    , currentChannel(currentChannel)
    , nbChannels(nbChannels)
{
    ASSERT(nbChannels > 0);
    ASSERT(currentChannel > -1);
    ASSERT(currentChannel < nbChannels);

    radioStates.setName("RadioState");
    radioStates.setEnabled(recordStats);
    radioStates.record(initialState);
    radioChannels.setName("RadioChannel");
    radioChannels.setEnabled(recordStats);
    radioChannels.record(currentChannel);

    // allocate memory for one dimension
    swTimes = new simtime_t*[numRadioStates];

    // go through the first dimension and
    for (int i = 0; i < numRadioStates; i++) {
        // allocate memory for the second dimension
        swTimes[i] = new simtime_t[numRadioStates];
    }

    // initialize all matrix entries to 0.0
    for (int i = 0; i < numRadioStates; i++) {
        for (int j = 0; j < numRadioStates; j++) {
            swTimes[i][j] = 0;
        }
    }
}

Radio::~Radio()
{
    // delete all allocated memory for the switching times matrix
    for (int i = 0; i < numRadioStates; i++) {
        delete[] swTimes[i];
    }

    delete[] swTimes;
    swTimes = nullptr;
}

simtime_t Radio::switchTo(int newState, simtime_t_cref now)
{
    // state to switch to must be in a valid range, i.e. 0 <= newState < numRadioStates
    ASSERT(0 <= newState && newState < numRadioStates);

    // state to switch to must not be SWITCHING
    ASSERT(newState != SWITCHING);

    // return error value if newState is the same as the current state
    // if (newState == state) return -1;

    // return error value if Radio is currently switching
    if (state == SWITCHING) return -1;

    /* REGULAR CASE */

    // set the nextState to the newState and the current state to SWITCHING
    nextState = newState;
    int lastState = state;
    state = SWITCHING;
    radioStates.record(state);

    // return matching entry from the switch times matrix
    return swTimes[lastState][nextState];
}

void Radio::setSwitchTime(int from, int to, simtime_t_cref time)
{
    // assert parameters are in valid range
    ASSERT(time >= 0.0);
    ASSERT(0 <= from && from < numRadioStates);
    ASSERT(0 <= to && to < numRadioStates);

    // it shall not be possible to set times to/from SWITCHING
    ASSERT(from != SWITCHING && to != SWITCHING);

    swTimes[from][to] = time;
    return;
}

void Radio::endSwitch(simtime_t_cref now)
{
    // make sure we are currently switching
    ASSERT(state == SWITCHING);

    // set the current state finally to the next state
    state = nextState;
    radioStates.record(state);

    return;
}
