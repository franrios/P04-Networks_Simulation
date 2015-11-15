/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_paquetes = 0;
  m_paquete_rechazado = 0;
}


void
Observador::PaqueteAsentido (Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  m_paquetes ++;
}

void
Observador::PaqueteRechazado (Ptr<const Packet> paquete_rechazado)
{
	NS_LOG_FUNCTION(paquete_rechazado);
	m_paquete_rechazado++; 
}


uint32_t
Observador::TotalPaquetes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_paquetes;
}

uint32_t
Observador::TotalPaquetesRechazados ()
{
	NS_LOG_FUNCTION_NOARGS();

	return m_paquete_rechazado;
}

double
Observador::GETCef (double probabilidad_error, uint32_t tamPkt, double rprop, double vtx)
{
	double cadenciaEficaz=(1-probabilidad_error)*((tamPkt*8)/((tamPkt+8)*8+probabilidad_error*2*rprop*vtx))*vtx;
	return cadenciaEficaz;
}

double
Observador::GETRend (double cadenciaEficaz, double vtx)
{
	double rendimiento = (cadenciaEficaz/vtx)*100;
	return rendimiento;
}
