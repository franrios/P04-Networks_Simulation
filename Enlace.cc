/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "Enlace.h"
#include "CabEnlace.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Enlace");

Enlace::Enlace(Ptr<NetDevice> disp,
                                 Time           espera,
                                 uint32_t       tamPqt,
                                 uint8_t        tamTx):m_ventana(tamTx,(uint32_t)RANGO)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt << tamTx);
  //
  //
  // QUITAR LAS VARIABLES INUTILES DEBIDO A VENTANA.H
  //
  //
  // Inicializamos las variables privadas
  m_disp      = disp;
  m_esperaACK = espera;
  m_tamPqt    = tamPqt;
  m_tx        = 0;
  m_rx        = 0;
  m_totalPqt  = 0;
  m_tamTx=tamTx;  //Valor de la ventana de tx
  m_totalPqtACK=0;
}

void
Enlace::ACKRecibido(uint8_t numSecuencia)
{
  //Ventana m_ventana (m_tamTx, rango);
 
  // Comprobamos si el número de secuencia del ACK se corresponde con
  // el de secuencia del siguiente paquete a transmitir
  //NS_LOG_INFO("ACK ha sido recibido ---------------------...--------------...---------------...------------");
  if(m_ventana.EnVentana(numSecuencia))
    {
     // NS_LOG_INFO("He recibido un ACK que se corresponde con el que esperaba--------------------------");
      // Si es correcto desactivo el temporizador
      Simulator::Cancel(m_temporizador);
      // aumentamos el número de secuencia
      m_ventana.Asentida(numSecuencia);
      // Incrementamos el total de paquetes
      m_totalPqt++;

      EnviaPaquete();
    }
}

void
Enlace::DatoRecibido(uint32_t numSecuencia)
{
  // Obtengo el valor del número de secuecia
  // ............................................................................
 // recibido->CopyData(&contenido,1);
  NS_LOG_DEBUG ("Recibido paquete en nodo " << m_node->GetId() << " con "
                << (unsigned int) numSecuencia);
  // Si el número de secuencia es correcto
      if(m_rx==numSecuencia) 
      {
         m_rx=(m_rx+1)%RANGO;
      }
      // Transmito en cualquier caso un ACK con el número de secuencia que espero recibir
      NS_LOG_INFO("Envío un ACK");
      EnviaACK();
}

void
Enlace::PaqueteRecibido(Ptr<NetDevice>        receptor,
                                 Ptr<const Packet>     recibido,
                                 uint16_t              protocolo,
                                 const Address &       desde,
                                 const Address &       hacia,
                                 NetDevice::PacketType tipoPaquete)
{
  NS_LOG_FUNCTION (receptor << recibido->GetSize () <<
                   std::hex << protocolo <<
                   desde << hacia << tipoPaquete);

  Ptr<Packet> copia = recibido->Copy(); //(Copy on write, COW) copia COW del paquete recibido
  CabEnlace header; 
  copia->RemoveHeader(header); //La cabecera se asocia a header

  uint8_t tipo = header.GetTipo(); //Obtenemos el tipo del paquete recibido
  uint8_t numSecuencia = header.GetSecuencia();

  if(tipo == 0)
    DatoRecibido(numSecuencia);
  else
    ACKRecibido(numSecuencia);
}

void
Enlace::VenceTemporizador()
{
  NS_LOG_FUNCTION_NOARGS ();
  //ESTE LOG HAY QUE MODIFICARLO
  NS_LOG_DEBUG ("NODO " << m_node->GetId() <<": VENCE TEMPORIZADOR" );
  //  << "Se reenvian los paquetes con numero de secuencia perteneciente al intervalo: ["
  //  << (unsigned int) (m_ventIni)%RANGO << "," << (m_ventIni + m_tamTx - 1)%RANGO << "].");

  //Iniciamos la ventana de transmision.
  m_ventana.Vacia();
  // Se reenvia un paquete
  EnviaPaquete();

  NS_LOG_DEBUG("NODO " << m_node->GetId() << ": SE RECUPERA DE ERROR");
}


void
Enlace::EnviaPaquete()
{
  NS_LOG_INFO("Tamaño del paquete " << m_tamPqt);
  NS_LOG_INFO ("Valor de m_tx: " << (unsigned int) m_tx);
 // NS_LOG_INFO ("Valor de ventana pendiente: " << (unsigned int) m_ventana.Pendiente());
  while(m_ventana.Credito()>0){
  Ptr<Packet> paquete = Create<Packet> (m_tamPqt);

  NS_LOG_FUNCTION_NOARGS ();

  CabEnlace header;

  header.SetData(0,m_tx);

  paquete->AddHeader(header);

  // Envío el paquete
  m_node->GetDevice(0)->Send(paquete, m_disp->GetAddress(), 0x0800);
  m_tx=m_ventana.Pendiente();


  //m_totalPqt++;

  NS_LOG_INFO ("   Transmitido paquete de " << paquete->GetSize () <<
               " octetos en nodo " << m_node->GetId() <<
               " con " << (unsigned int) m_tx <<
               " en " << Simulator::Now());
  // Programo el temporizador
  if (m_esperaACK != 0 && m_temporizador.IsRunning()== false)
    m_temporizador=Simulator::Schedule(m_esperaACK, &Enlace::VenceTemporizador,this);
  }
  if(m_ventana.Credito()==0)    
    NS_LOG_DEBUG("NODO " << m_node->GetId() << ": LLENA LA VENTANA");
}

void
Enlace::EnviaACK()
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> p = Create<Packet> (1);

  //Formamos la cabecera
  CabEnlace header;
  header.SetData (1, m_rx);
  //Añadimos la cabecera
  p->AddHeader (header);

    m_node->GetDevice(0)->Send(p, m_disp->GetAddress(), 0x0800);

  NS_LOG_DEBUG ("Transmitido ACK de " << p->GetSize () <<
                " octetos en nodo " << m_node->GetId() <<
                " con " << (unsigned int) m_rx <<
                " en " << Simulator::Now());

}
