/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include <ns3/random-variable-stream.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>
#include <sstream>
#include <ns3/average.h>
#include "Enlace.h"
#include "Observador.h"

using namespace ns3;

#define TSTUDENT 1.8331

NS_LOG_COMPONENT_DEFINE ("Practica04");


double
simulacion (Time espera, uint32_t pktSize, Time retardo, DataRate tasa, uint32_t ventana, double prob_error)
{

  // Parámetros de la simulación
  Time     trtx       = espera;
  uint32_t tamPaquete = pktSize;
  Time     rprop      = retardo;
  DataRate vtx        = tasa;
  uint32_t  tamVentana = ventana;
  double   probabilidad_error     = prob_error;
  double   cadenciaEficaz = 0.0;
  double   rendimiento = 0.0;



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
  error_model->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);


  // Creamos los nodos
  NodeContainer      nodos;
  nodos.Create(2);
  // Creamos el escenario
  NetDeviceContainer dispositivos = escenario.Install (nodos);

  // Una aplicación transmisora
  Enlace transmisor (dispositivos.Get (1), trtx, tamPaquete, tamVentana);
  // Y una receptora 
  Enlace receptor (dispositivos.Get (0), trtx, tamPaquete, tamVentana);

  dispositivos.Get(0)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_model);
  dispositivos.Get(1)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_model);

  //Creamos un objeto de tipo observador para poder contabilizar los paquetes asentidos y rechazados, la cadencia y el rendimiento
  Observador observador;

  // Suscribimos la traza de paquetes correctamente asentidos.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));
  // y también para los rechazados
  dispositivos.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteRechazado, &observador));

  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&transmisor);
  nodos.Get (1)->AddApplication(&receptor);

  //Añadimos una salida a pcap
  //escenario.EnablePcap("practica04", dispositivos.Get(0));

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (6.0));
  // Activamos el transmisor
  receptor.SetStartTime (Seconds (1.0));
  receptor.SetStopTime (Seconds (6.0));

  Simulator::Run ();
  Simulator::Destroy ();

  //Calculamos la cadencia eficaz y el rendimiento.
  cadenciaEficaz = observador.GETCef(probabilidad_error, tamPaquete, rprop.GetDouble()*1e-6, vtx.GetBitRate());
  rendimiento = observador.GETRend(cadenciaEficaz, vtx.GetBitRate());

  NS_LOG_INFO  (" ");
  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 8);
  NS_LOG_INFO  ("----------------------------");
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << Seconds(vtx.CalculateTxTime (tamPaquete + 8)) + 2 * rprop);
  NS_LOG_DEBUG ("Temporizador: " << trtx);
  NS_LOG_DEBUG ("Probabilidad de error de bit: " << probabilidad_error);
  NS_LOG_INFO  ("----------------------------");
  NS_LOG_INFO  ("Total paquetes: " << observador.TotalPaquetes ());
  NS_LOG_INFO  ("Total paquetes erroneos: " << observador.TotalPaquetesRechazados ());
  NS_LOG_INFO  ("----------------------------");
  NS_LOG_INFO  ("Cadencia eficaz: " << cadenciaEficaz << "bps");
  NS_LOG_INFO  ("Rendimiento: " << rendimiento << "(%)");
  NS_LOG_INFO  (" ");


  return rendimiento;
}

int
main (int argc, char *argv [])
{
  Time::SetResolution (Time::US);

  // Parámetros de la simulación
  Time     trtx       = Time("6ms");
  uint32_t tamPaquete = 121;
  Time     rprop      = Time("0.2ms");
  DataRate vtx        = DataRate("1Mbps");
  uint32_t  tamVentana = 6;
  double   probabilidad_error     = 0;
  double z =0.0;
  double numero_simulaciones=10;

  Average<double> ac_rendimiento; //Acumulador utilizado para obtener las medias y varianzas

  CommandLine cmd;
  
  cmd.AddValue("window","Tamaño de la ventana de transmisión.",tamVentana);
  cmd.AddValue("delay","Retardo de propagación del enlace.",rprop);
  cmd.AddValue("rate","Capacidad de transmisión en el canal.",vtx);
  cmd.AddValue("pktSize","Tamaño de la SDU del nivel de enlace.",tamPaquete);
  cmd.AddValue("wait","Tiempo de espera para la retransmisión.",trtx);
  cmd.AddValue("numSimulaciones","Número de simulaciones para pruebas",numero_simulaciones);

  cmd.Parse(argc,argv);


  //Preparamos la gráfica 
  Gnuplot plot;
  plot.SetTitle("Práctica 04 - Rendimiento frente a prob. error de paquete");
  plot.SetLegend("prob. error de paquete", "rendimiento en %");

  std::stringstream sstm;
  sstm << "Evolución del rendimiento";
  std::string curva = sstm.str();

  Gnuplot2dDataset datos;
  datos.SetTitle(curva);
  datos.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  datos.SetErrorBars (Gnuplot2dDataset::Y);

  for (probabilidad_error=0.0;probabilidad_error<=0.5;probabilidad_error=probabilidad_error+0.05)
  {
    for (int i = 0; i < numero_simulaciones; i++)
    {
      ac_rendimiento.Update(simulacion(trtx, tamPaquete, rprop, vtx, tamVentana, probabilidad_error));

    }

    z = TSTUDENT*sqrt(ac_rendimiento.Var()/(numero_simulaciones));
    datos.Add(probabilidad_error, ac_rendimiento.Mean(), z); //Para generar puntos en la gráfica
    ac_rendimiento.Reset(); //Para volver a iterar y seguir acumulando
  }

/*Para terminar de formar la gráfica, añadimos todos los datos recogidos
  y generamos el fichero de salida "practica04.plt"*/
  plot.AddDataset(datos);
  std::ofstream fichero("practica04.plt");
  plot.GenerateOutput(fichero);
  fichero << "pause -1" << std::endl;
  fichero.close();
}



