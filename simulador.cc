/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include <ns3/random-variable-stream.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/average.h>
#include <sstream>
#include <ns3/command-line.h>
#include "Enlace.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica04");

double
simulacion(Time espera_rtx, uint32_t pktSize, Time retardo_prop, DataRate v_trans, uint8_t tamanio_Ventana, double p_error)
{
    // Parámetros de la simulación
  Time     trtx             = espera_rtx;
  uint32_t tamPaquete       = pktSize;
  Time     rprop            = retardo_prop;
  DataRate vtx              = v_trans;
  uint8_t  tamVentana       = tamanio_Ventana;
  double probabilidad_error = p_error;
  double cadenciaEficaz=0.0;
  double rendimiento=0.0;

  // Configuramos el escenario:
  PointToPointHelper escenario;
  escenario.SetChannelAttribute ("Delay", TimeValue (rprop));
  escenario.SetDeviceAttribute ("DataRate", DataRateValue (vtx));
  escenario.SetQueue ("ns3::DropTailQueue");

  //Añadimos el error
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

//Creamos un objeto de tipo observador para poder contabilizar los paquetes asentidos y rechazados
  Observador observador;

  // Suscribimos la traza de paquetes correctamente asentidos.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));
  //Suscribimos la traza de paquetes rechazados.
  dispositivos.Get (1)->GetObject<PointToPointNetDevice>()->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteRechazado, &observador));

  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&transmisor);
  nodos.Get (1)->AddApplication(&receptor);

  //Para poder visualizar la simulación con wireshark:
 // escenario.EnablePcap("pcapPractica4",dispositivos.Get(0));

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (6.0)); //cambiado 9.95 por 1.95 para pruebas

  receptor.SetStartTime(Seconds (1.0));
  receptor.SetStopTime(Seconds (6.0));
  
  Simulator::Run ();
  Simulator::Destroy ();

    //Calculamos la cadencia eficaz y el rendimiento.
  cadenciaEficaz = observador.GETCef(probabilidad_error, tamPaquete, rprop.GetDouble()*1e-6, vtx.GetBitRate());
  rendimiento = observador.GETRend(cadenciaEficaz, vtx.GetBitRate());

  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 8); //Revisar esta parte
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << vtx.CalculateTxTime (tamPaquete + 6) + 2 * rprop);
  NS_LOG_DEBUG ("Temporizador: " << trtx);
  NS_LOG_INFO  ("Total paquetes: " << observador.TotalPaquetes ());
  NS_LOG_INFO ("Paquetes erróneos: " << observador.TotalPaquetesRechazados());
  NS_LOG_INFO ("Cadencia eficaz: " << cadenciaEficaz);
  NS_LOG_INFO ("Rendimiento: " << rendimiento);


  return 0;
}


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
  double probabilidad_error = 0.0;
  double pruebas=0;

//  Average<double> ac_rendimiento;


  /*Preparamos los parámetros solicitados para poder ser introducidos
    opcionalmente por línea de comandos*/
  CommandLine cmd;

  cmd.AddValue("windows","Tamaño de la ventana de transmisión",tamVentana);
  cmd.AddValue("delay","Retardo de propagación del enlace",rprop);
  cmd.AddValue("rate","Capacidad de transmisión del canal",vtx);
  cmd.AddValue("pktSize","Tamaño de la SDU de nivel de enlace",tamPaquete);
  cmd.AddValue("wait","Tiempo de espera para la retransmisión",trtx);

  cmd.Parse(argc,argv);

//for(probabilidad_error=0.0; probabilidad_error<=0.5; probabilidad_error=probabilidad_error+0.05)
//{
 // for(int i=0;i<10;i++)
  //{
   // ac_rendimiento.Update(simulacion(trtx,tamPaquete,rprop,vtx,tamVentana,probabilidad_error));
  pruebas=simulacion(trtx,tamPaquete,rprop,vtx,tamVentana,probabilidad_error);
  NS_LOG_INFO ("Pruebas vale: " << pruebas);
  //}
  //NS_LOG_INFO("La media de las 10 simulaciones de ac_rendimiento es:" << ac_rendimiento.Mean() << "Para una probabilidad_error de " << probabilidad_error);
  //ac_rendimiento.Reset();
//}


//return 0;

}
