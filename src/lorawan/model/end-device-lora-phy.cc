/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include <algorithm>
#include "ns3/end-device-lora-phy.h"
#include "ns3/simulator.h"
#include "ns3/lora-tag.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("EndDeviceLoraPhy");

NS_OBJECT_ENSURE_REGISTERED (EndDeviceLoraPhy);

/**************************
 *  Listener destructor  *
 *************************/

EndDeviceLoraPhyListener::~EndDeviceLoraPhyListener ()
{
}

TypeId
EndDeviceLoraPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EndDeviceLoraPhy")
    .SetParent<LoraPhy> ()
    .SetGroupName ("lorawan")
    .AddTraceSource ("LostPacketBecauseWrongFrequency",
                     "Trace source indicating a packet "
                     "could not be correctly decoded because"
                     "the ED was listening on a different frequency",
                     MakeTraceSourceAccessor (&EndDeviceLoraPhy::m_wrongFrequency),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("LostPacketBecauseWrongSpreadingFactor",
                     "Trace source indicating a packet "
                     "could not be correctly decoded because"
                     "the ED was listening for a different Spreading Factor",
                     MakeTraceSourceAccessor (&EndDeviceLoraPhy::m_wrongSf),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("EndDeviceState",
                     "The current state of the device",
                     MakeTraceSourceAccessor
                       (&EndDeviceLoraPhy::m_state),
                     "ns3::TracedValueCallback::EndDeviceLoraPhy::State");
  return tid;
}

// Initialize the device with some common settings.
// These will then be changed by helpers.
EndDeviceLoraPhy::EndDeviceLoraPhy () :
  m_state (SLEEP),
  m_frequency (868.1),
  m_sf (7),
  m_CSMAx (MilliSeconds (10))
{
}

EndDeviceLoraPhy::~EndDeviceLoraPhy ()
{
}

// Downlink sensitivity (from SX1272 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivites are for a bandwidth of 125000 Hz
const double EndDeviceLoraPhy::sensitivity[6] =
{-124, -127, -130, -133, -135, -137};

void
EndDeviceLoraPhy::SetSpreadingFactor (uint8_t sf)
{
  m_sf = sf;
}

uint8_t
EndDeviceLoraPhy::GetSpreadingFactor (void)
{
  return m_sf;
}

bool
EndDeviceLoraPhy::IsTransmitting (void)
{
  return m_state == TX;
}

bool
EndDeviceLoraPhy::IsOnFrequency (double frequencyMHz)
{
  return m_frequency == frequencyMHz;
}

bool
EndDeviceLoraPhy::IsChannelOccupied (double frequency)
{
  // start- and endtime of CCG (Clear Channel Gap)
  Time ccgStart = Simulator::Now ();
  Time ccgEnd = ccgStart + m_CSMAx;

  int occupierCount = 0;
  // Copy the interferers list from LoraInterferenceHelper
  std::list< Ptr< LoraInterferenceHelper::Event > > interferers = m_interference.GetInterferers ();
  for (auto ite = interferers.begin (); ite != interferers.end (); ++ite)
  {
    uint8_t sf = (*ite)->GetSpreadingFactor ();

    /**
     * If there are events in the list, for which their start time is later than
     * the end time of the CCG, or, if their end time is earlier than the start
     * time of the CCG, they are not considered. Otherwise, these events are con-
     * sidered as possible occupiers and investigated further for reception power.
     */
    if ((*ite)->GetFrequency () == frequency &&
        !((*ite)->GetEndTime () <= ccgStart ||
         ccgEnd < (*ite)->GetStartTime ()))
    {
      // Checking the reception power against the sensitivity thresholds
      if ((*ite)->GetRxPowerdBm () > sensitivity[sf - 7])
      {
        ++occupierCount;
        NS_LOG_DEBUG ("Occupier found for frequency: " << (*ite)->GetFrequency () <<
                         "MHz, RxPower: " << (*ite)->GetRxPowerdBm () << " dBm " <<
                         "@ SF" << (unsigned int) sf << " (threshold = " <<
                         sensitivity[sf - 7] << " dBm), eventStart: " <<
                         (*ite)->GetStartTime ().GetSeconds () << ", eventEnd: " <<
                         (*ite)->GetEndTime ().GetSeconds () << ", ccgStart: " <<
                         ccgStart.GetSeconds () << ", ccgEnd: " <<
                         ccgEnd.GetSeconds ());
      }
    }
  }
  return (occupierCount != 0);
}

void
EndDeviceLoraPhy::SetFrequency (double frequencyMHz)
{
  m_frequency = frequencyMHz;
}

void
EndDeviceLoraPhy::SwitchToStandby (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_state = STANDBY;

  // Notify listeners of the state change
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyStandby ();
    }
}

void
EndDeviceLoraPhy::SwitchToRx (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state == STANDBY);

  m_state = RX;

  // Notify listeners of the state change
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart ();
    }
}

void
EndDeviceLoraPhy::SwitchToTx (double txPowerDbm)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state != RX);

  m_state = TX;

  // Notify listeners of the state change
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (txPowerDbm);
    }
}

void
EndDeviceLoraPhy::SwitchToSleep (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state == STANDBY);

  m_state = SLEEP;

  // Notify listeners of the state change
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifySleep ();
    }
}

EndDeviceLoraPhy::State
EndDeviceLoraPhy::GetState (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_state;
}

void
EndDeviceLoraPhy::RegisterListener (EndDeviceLoraPhyListener *listener)
{
  m_listeners.push_back (listener);
}

void
EndDeviceLoraPhy::UnregisterListener (EndDeviceLoraPhyListener *listener)
{
  ListenersI i = find (m_listeners.begin (), m_listeners.end (), listener);
  if (i != m_listeners.end ())
    {
      m_listeners.erase (i);
    }
}

}
}
