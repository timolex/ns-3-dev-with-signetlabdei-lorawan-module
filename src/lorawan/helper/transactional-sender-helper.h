/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TRANSACTIONAL_SENDER_HELPER_H
#define TRANSACTIONAL_SENDER_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/transactional-sender.h"
#include <stdint.h>
#include <string>

namespace ns3 {
namespace lorawan {

/**
 * This class can be used to install TransactionalSender applications on a wide
 * range of nodes.
 */
class TransactionalSenderHelper
{
public:
  TransactionalSenderHelper ();

  ~TransactionalSenderHelper ();

  void SetAttribute (std::string name, const AttributeValue &value);

  ApplicationContainer Install (NodeContainer c) const;

  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Set the period to be used by the applications created by this helper.
   *
   * A value of Seconds (0) results in randomly generated periods according to
   * the model contained in the TR 45.820 document.
   *
   * \param period The period to set
   */
  void SetPeriod (Time period);

  void SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv);

  void SetPacketSize (uint8_t size);

  void SetDataPacketSize (uint8_t dataSize);

  uint8_t GetDataPacketSize (void) const;

  void SetPartialSignaturePacketSize (uint8_t sigSize);

  uint8_t GetPartialSignaturePacketSize (void) const;

  void SetPacketsPerTransaction (uint32_t packets);

  uint32_t GetPacketsPerTransaction (void) const;

  /**
   * Set the intra-transaction-delay
   */
  void SetIntraTransactionDelay (Time intraDelay);

  /**
   * Get the intra-transaction-delay
   */
  Time GetIntraTransactionDelay (void) const;

  /**
   * Set the inter-transaction-delay
   */
  void SetInterTransactionDelay (Time interDelay);

  /**
   * Get the inter-transaction-delay
   */
  Time GetInterTransactionDelay (void) const;


private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

  Ptr<UniformRandomVariable> m_initialDelay;

  Ptr<UniformRandomVariable> m_intervalProb;

  Time m_period; //!< The period with which the application will be set to send
                 // messages

  Ptr<RandomVariableStream> m_pktSizeRV; // whether or not a random component is added to the packet size

  uint8_t m_pktSize; // the packet size.

  /**
   * The size of regular data packets
   */
  uint8_t dataPktSize;

  /**
   * The size of one of two signature packets
   */
  uint8_t sigPartPktSize;

  /**
   * The number of packets per transaction
   */
  uint32_t packetsPerTransaction;

  /**
   * The interval between two consecutive transactions
   */
  Time interTransactionDelay;

  /**
   * The interval between the transmissions of two
   * consecutive packets belonging to a transaction.
   */
  Time intraTransactionDelay;

};

} // namespace ns3

}
#endif /* TRANSACTIONAL_SENDER_HELPER_H */
