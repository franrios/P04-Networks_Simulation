/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

using namespace ns3;

#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"
#include "Ventana.h"

#define RANGO 256

class Enlace : public Application
{
public:

  // Constructor de la clase. Necesita como parámetros el puntero al dispositivo de red
  // con el que debe comunicarse, el temporizador de retransmisiones y el tamaño de
  // paquete. Inicializa las variables privadas.
  Enlace(Ptr<NetDevice>, Time, uint32_t tamPqt, uint8_t tamTx);



  // Función que envía un paquete.
  void EnviaACK();


  // Función para el procesamiento de asentimientos recibidos.
  // Comprueba si el ACK es el adecuado. Si lo es, desactiva el temporizador de
  // retransmisiones, actualiza el valor de la ventana y envía un nuevo paquete.
  void ACKRecibido(uint8_t numSecuencia);
  void DatoRecibido(uint32_t numSecuencia);

  // Función para el procesamiento de paquetes recibidos
  // Comprueba si el ACK es el adecuado. Si lo es, desactiva el temporizador de
  // retransmisiones, actualiza el valor de la ventana y envía un nuevo paquete.
  void PaqueteRecibido(Ptr<NetDevice> receptor, Ptr<const Packet> recibido,
                       uint16_t protocolo, const Address &desde, const Address &hacia,
                       NetDevice::PacketType tipoPaquete);

  // Función de vencimiento del temporizador
  void VenceTemporizador ();
  
  // Función que envía un paquete.
  void EnviaPaquete();

  // Función que obtiene el total de paquetes correctamente recibidos por el receptor.
  //uint32_t TotalDatos();

private:
  // Método de inicialización de la aplicación.
  // Se llama sólo una vez al inicio.
  // En nuestro caso sirve para instalar el Callback que va a procesar
  // los asentimientos recibidos.
  void DoInitialize()
  {
    // Solicitamos que nos entreguen (mediante la llamada a ACKRecibido)
    // cualquier paquete que llegue al nodo.
    m_node->RegisterProtocolHandler (ns3::MakeCallback(&Enlace::PaqueteRecibido,
                                                       this),
                                     0x0000, 0, false);
    Application::DoInitialize();
  };

  // Método que se llama en el instante de comienzo de la aplicación.
  void StartApplication()
  {
          m_ventana.Vacia();
          EnviaPaquete();
        //  m_tx=m_ventana.Pendiente();
       // }
  }

  // Método que se llama en el instante de final de la aplicación.
  void StopApplication()
  {
    Simulator::Stop ();
  }

  // Dispositivo de red con el que hay que comunicarse.
  Ptr<NetDevice> m_disp;
  // Temporizador de retransmisión
  Time           m_esperaACK;
  // Tamaño del paquete
  uint32_t       m_tamPqt;
  // Número de secuencia de los paquetes a transmitir 
  uint8_t        m_tx;

  //num secuencia inicial de la ventana V(A)
 // uint8_t        m_vent_ini;
  
  //tam ventana tx (k)
  uint8_t        m_tamTx;

  // Evento de retransmision
  EventId        m_temporizador;
  // Acumulador de paquetes bien asentidos.
  int            m_totalPqt;

  // Paquete para ser enviado 
  Ptr<Packet>    m_paquete;

  // Número de secuencia de los paquetes a recibir

  //V(R) en el estándar (número secuencial de la siguiente trama que debe recibirse)
  uint8_t        m_rx;

  uint32_t       rango;

//Contabilizar paquetes asentidos
  int m_totalPqtACK;

 // Ventana m_ventana;
  Ventana m_ventana;
};
