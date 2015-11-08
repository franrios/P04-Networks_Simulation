/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>

using namespace ns3;


class Observador
{
public:
  Observador  ();
  void     PaqueteAsentido (Ptr<const Packet> paquete);
  void	   PaqueteRechazado (Ptr<const Packet> paquete_rechazado);
  uint32_t TotalPaquetes   ();

private:
  uint64_t m_paquetes;
  uint64_t m_paquete_rechazado;
};
