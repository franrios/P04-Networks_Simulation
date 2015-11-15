/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include <ns3/random-variable-stream.h>
#include <ns3/point-to-point-net-device.h>
#include "Enlace.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica04");


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::US);


  /*tamaño de la ventana de transmisión, window (6)
retardo de propagación del enlace, delay (0,2 ms)
capacidad de transmisión en el canal, rate (1Mbit/s)
tamaño de la SDU del nivel de enlace, pktSize (121 octetos)
tiempo de espera para la retransmisión, wait (6 ms)*/


  // Parámetros de la simulación
  Time     trtx             = Time("6ms");
  uint32_t tamPaquete       = 121;
  Time     rprop            = Time("0.2ms");
  DataRate vtx              = DataRate("1Mbps");
  uint8_t  tamVentana       = 6;
  double probabilidad_error = 0.00002;

  
  /*Preparamos los parámetros solicitados para poder ser introducidos
    opcionalmente por línea de comandos*/
  CommandLine cmd;

  cmd.AddValue("window","Tamaño de la ventana de transmisión",tamVentana);
  cmd.AddValue("delay","Retardo de propagación del enlace",rprop);
  cmd.AddValue("rate","Capacidad de transmisión del canal",vtx);
  cmd.AddValue("pktSize","Tamaño de la SDU de nivel de enlace",tamPaquete);
  cmd.AddValue("wait","Tiempo de espera para la retransmisión",trtx);

  cmd.Parse(argc,argv);

  //Ventana ventana (tamVentana,256);

  // Configuramos el escenario:
  PointToPointHelper escenario;
  escenario.SetChannelAttribute ("Delay", TimeValue (rprop));
  escenario.SetDeviceAttribute ("DataRate", DataRateValue (vtx));
  escenario.SetQueue ("ns3::DropTailQueue");


  Ptr<RateErrorModel> error_model = CreateObject<RateErrorModel>();
  Ptr<UniformRandomVariable> distribucion_del_error = CreateObject<UniformRandomVariable>();

  error_model->SetRandomVariable(distribucion_del_error);
  error_model->SetRate(probabilidad_error);
  error_model->SetUnit(RateErrorModel::ERROR_UNIT_BIT);

  // Creamos los nodos
  NodeContainer      nodos;
  nodos.Create(2);
  // Creamos el escenario
  NetDeviceContainer dispositivos = escenario.Install (nodos);


    // Una aplicación transmisora
  Enlace transmisor (dispositivos.Get (1), trtx, tamPaquete, tamVentana);
  // Y una receptora
  Enlace receptor(dispositivos.Get (0), trtx, tamPaquete, tamVentana);

  dispositivos.Get(0)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_model);
  dispositivos.Get(1)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_model);

  Observador observador;
  // Suscribimos la traza de paquetes correctamente asentidos.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));
  dispositivos.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteRechazado, &observador));


  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&transmisor);
  nodos.Get (1)->AddApplication(&receptor);

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (1.05)); //cambiado 9.95 por 1.95 para pruebas

  receptor.SetStartTime(Seconds (1.0));
  receptor.SetStopTime(Seconds (1.05));
  
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 8); //Revisar esta parte
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << vtx.CalculateTxTime (tamPaquete + 6) + 2 * rprop);
  NS_LOG_DEBUG ("Temporizador: " << trtx);
  NS_LOG_INFO  ("Total paquetes: " << observador.TotalPaquetes ());
  NS_LOG_INFO ("Paquetes erróneos: " << observador.TotalPaquetesRechazados());
  
  return 0;
}
