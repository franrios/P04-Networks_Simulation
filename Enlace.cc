//Esto es solo una prueba para git

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "Enlace.h"
#include "CabEnlace.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BitAlternante");

Enlace::Enlace(Ptr<NetDevice> disp,
                                 Time           espera,
                                 uint32_t       tamPqt,
                                 uint8_t        tamTx)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt);

  // Inicializamos las variables privadas
  m_disp      = disp;
  m_esperaACK = espera;
  m_tamPqt    = tamPqt;
  m_tx        = 0;
  m_rx        = 0;
  m_totalPqt  = 0;
  m_tamTx=tamTx;  //Valor de la ventana de tx
  m_vent_ini=0;
  m_totalPqtACK=0;

  NS_LOG_INFO("Parámetros del inicio: ventana " << (unsigned int) m_tamTx << "evaluado " << m_vent_ini + m_tamTx - 1);
}



void
Enlace::ACKRecibido(uint8_t numSecuencia)
{
 
  // Comprobamos si el número de secuencia del ACK se corresponde con
  // el de secuencia del siguiente paquete a transmitir
  // .....................................................................
  NS_LOG_INFO("ACK ha sido recibido ---------------------...--------------...---------------...------------");
  if(numSecuencia==(m_vent_ini+1)%256)
    {
      NS_LOG_INFO("He recibido un ACK que se corresponde con el que esperaba--------------------------");
      // Si es correcto desactivo el temporizador
      // ....................
      Simulator::Cancel(m_temporizador);
      // aumentamos el número de secuencia
      // ...................................................................
      m_vent_ini++;
      // Incrementamos el total de paquetes
      // ...................................................................
      //m_totalPqt++;
      // Se transmite un nuevo paquete
      if(m_tx >= m_vent_ini && m_vent_ini <= m_vent_ini + m_tamTx - 1)
      {
        //Si el número de secuencia del próximo paquete a tx (V(S)) se encuentra en el rango que abarca la ventana de tx, se envía
        //m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
        EnviaPaquete();
        m_tx ++;

      }

    }
    else
    {
      NS_LOG_INFO("El ACK que se esperaba corresponde con: " << (unsigned int) m_vent_ini+1 << " y se ha recibido " << (unsigned int) numSecuencia);
      Simulator::Cancel(m_temporizador);
      VenceTemporizador();
    }
}


void
Enlace::VenceTemporizador()
{
  NS_LOG_FUNCTION_NOARGS ();
  // Reenviamos el último paquete transmitido
  // ...........................................
     // m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
    //  EnviaPaquete();
    NS_LOG_ERROR ("Se ha producido una retransmisión.");

  for (m_tx = m_vent_ini; m_tx >= m_vent_ini && m_tx < m_vent_ini + m_tamTx; m_tx++)
  {
    // Se reenvia un paquete
    EnviaPaquete();
  }
}


void
Enlace::EnviaPaquete()
{
  NS_LOG_INFO("Tamaño del paquete " << m_tamPqt);

  Ptr<Packet> paquete = Create<Packet> (m_tamPqt);

  NS_LOG_FUNCTION_NOARGS ();

  CabEnlace header;

  header.SetData(0,m_tx);

  paquete->AddHeader(header);

  // Envío el paquete
  m_node->GetDevice(0)->Send(paquete, m_disp->GetAddress(), 0x0800);

  m_totalPqt++;

  NS_LOG_INFO ("   Transmitido paquete de " << paquete->GetSize () <<
               " octetos en nodo " << m_node->GetId() <<
               " con " << (unsigned int) m_tx <<
               " en " << Simulator::Now());
  // Programo el temporizador
  if (m_esperaACK != 0 && m_temporizador.IsRunning()== false)
    m_temporizador=Simulator::Schedule(m_esperaACK, &Enlace::VenceTemporizador,this);
}

/*
uint32_t
BitAlternanteTx::TotalDatos()
{
  // Devuelvo el total de paquetes enviados
  // ......................................................
  return this->m_totalPqt;
}
*/



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
 // uint8_t contenido;

  Ptr<Packet> copia = recibido->Copy(); //(Copy on write, COW) copia COW del paquete recibido
  CabEnlace header; 
  copia->RemoveHeader(header); //La cabecera se asocia a header

  uint8_t tipo = header.GetTipo(); //Obtenemos el tipo del paquete recibido
  uint8_t numSecuencia = header.GetSecuencia();
NS_LOG_INFO("..............................................................................");

  if(tipo == 0)
    DatoRecibido(numSecuencia);
  else
    ACKRecibido(numSecuencia);
}

void
Enlace::DatoRecibido(uint8_t numSecuencia)
{
  // Obtengo el valor del número de secuecia
  // ............................................................................
 // recibido->CopyData(&contenido,1);
  NS_LOG_DEBUG ("Recibido paquete en nodo " << m_node->GetId() << " con "
                << (unsigned int) numSecuencia);
  // Si el número de secuencia es correcto
      if(m_rx==numSecuencia) 
      {
        // Si es correcto, aumento el número de secuencia
  NS_LOG_INFO("Recibido paquete de DATOS con numSec: " << (unsigned int) numSecuencia << ", y  V(R): " << (unsigned int) m_rx << "--------------------");

        m_rx++;
      }
      // Transmito en cualquier caso un ACK con el número de secuencia que espero recibir
      NS_LOG_INFO("Envío un ACK");
      EnviaACK();
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

  NS_LOG_DEBUG ("Transmitido ACK de " << p->GetSize () <<
                " octetos en nodo " << m_node->GetId() <<
                " con " << (unsigned int) m_rx <<
                " en " << Simulator::Now());
  m_node->GetDevice(0)->Send(p, m_disp->GetAddress(), 0x0800);
}
