#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/gnuplot.h"
using namespace ns3;

static uint16_t sinkPort = 8080;
uint32_t start_time_sink = 0;
uint32_t start_time_apps = 0.5;
uint32_t run_time = 10;
uint32_t packetSize = 1500;
uint32_t packetsToSend = 1000000;
uint32_t varDataRate = 20;

Gnuplot2dDataset dataset;
Gnuplot2dDataset datasetTcp;
Gnuplot2dDataset datasetUdp;
Gnuplot2dDataset datasetTcp_udprate;
Gnuplot2dDataset datasetUdp_udprate;
Gnuplot2dDataset datasetOtherUdp_udprate;
Gnuplot2dDataset datasetTcp1;
Gnuplot2dDataset datasetTcp2;
Gnuplot2dDataset datasetTcp3;
Gnuplot2dDataset datasetTcp4;
Gnuplot2dDataset datasetUdp1;
Gnuplot2dDataset datasetUdp2;

NS_LOG_COMPONENT_DEFINE("Networks_Lab_4");

class MyApp : public Application
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  void ChangeRate(DataRate newrate);


private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

//used to change the data-rate of the application
void MyApp::ChangeRate(DataRate newrate)
{
   m_dataRate = newrate;
   return;
}

//This function is used to increase the data rate of an application
void IncRate (Ptr<MyApp> app, DataRate rate, FlowMonitorHelper *flowMonitorHelp, Ptr<FlowMonitor> flowMonitor, int boolvar)
{
  app->ChangeRate(rate);
  if(boolvar==1)
  {
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowMonitorHelp->GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> fmStats = flowMonitor->GetFlowStats ();
	double totFlow = 0, totFlowSquare = 0;
	double TCPsum = 0, UDPsum=0, totOtherUDP=0;
	double TCPs[4];
	double UDPs[2];	
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = fmStats.begin (); i != fmStats.end (); ++i)
	{
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
		double throughPut = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/(1024*1024);
		totFlow += throughPut;
		totFlowSquare += throughPut * throughPut ;
		if(t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.6.2")
		{
		    TCPsum += throughPut;
		    TCPs[0] += throughPut;
		}
		else if(t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.7.2")
		{
		  TCPsum += throughPut;
		  TCPs[1] += throughPut;
		}
		else if(t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.7.2")
		{
	  		TCPsum += throughPut;
		    TCPs[2] += throughPut;
		}
		else if(t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.6.2"){
		TCPsum += throughPut;
		TCPs[3] += throughPut;
		}
		else if(t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.1.1" )
		{
			UDPsum += throughPut;
			totOtherUDP += throughPut;
		    UDPs[0] += throughPut;
		}
		else if(t.sourceAddress == "7.7.2.1" && t.destinationAddress == "7.7.3.1" )
		{
			UDPsum += throughPut;
		  UDPs[1] += throughPut;
		}
	}
	
	if(TCPsum!=0)datasetTcp_udprate.Add (varDataRate, TCPsum);
	if(UDPsum!=0)datasetUdp_udprate.Add (varDataRate, UDPsum);
	if(totOtherUDP!=0)datasetOtherUdp_udprate.Add (varDataRate, totOtherUDP); 
	
	datasetTcp1.Add (varDataRate, TCPs[0]);
	datasetTcp2.Add (varDataRate, TCPs[1]);
	datasetTcp3.Add (varDataRate, TCPs[2]);
	datasetTcp4.Add (varDataRate, TCPs[3]);
	datasetUdp1.Add (varDataRate, UDPs[0]);
	datasetUdp2.Add (varDataRate, UDPs[1]);

	std :: cout << "UDP Data Rate: " << varDataRate << "Mbps,\t\t";
	std :: cout << "TCP Throughput: " << TCPsum << ",\t\t";
	std :: cout << "UDP Throughput: " << UDPsum << ",\t\t";
	std :: cout << "Remaining UDP Throughput: " << totOtherUDP << std :: endl;
  }
  varDataRate+=10;


}

//Function for establishing TCP connection between trace source and trace sink.
Ptr<MyApp> establishTCP(NodeContainer &nodes, Ipv4InterfaceContainer &intface, int source, int sink, int bsize)
{
  sinkPort++;  //new port
  Address sinkAddress (InetSocketAddress (intface.GetAddress (1), sinkPort));
  PacketSinkHelper pckSink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));

  ApplicationContainer sinkApps = pckSink.Install (nodes.Get (sink));
  sinkApps.Start (Seconds (start_time_sink));
  sinkApps.Stop (Seconds (run_time));
  
  Ptr<Socket> tcpSocket = Socket::CreateSocket (nodes.Get (source), TcpSocketFactory::GetTypeId ());  //Creating a tcp socket 
  tcpSocket->SetAttribute("SndBufSize",  ns3::UintegerValue(bsize));
  tcpSocket->SetAttribute("RcvBufSize",  ns3::UintegerValue(bsize));

  Ptr<MyApp> tcp_Agent = CreateObject<MyApp> ();			//Creating sender application
  tcp_Agent->Setup (tcpSocket, sinkAddress, packetSize, packetsToSend, DataRate ("20Mbps"));
  nodes.Get (source)->AddApplication (tcp_Agent);
  tcp_Agent->SetStartTime (Seconds (start_time_apps));
  tcp_Agent->SetStopTime (Seconds (run_time));	

  return tcp_Agent;
}

//Function for establishing UDP connection between trace source and trace sink.
Ptr<MyApp> establishUDP(NodeContainer &nodes, Ipv4InterfaceContainer &intface, int source, int sink, int bsize){

  sinkPort++;			//new port
  Address sinkAddress (InetSocketAddress (intface.GetAddress (0), sinkPort));
  PacketSinkHelper pckSink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));

  ApplicationContainer sinkApps = pckSink.Install (nodes.Get (sink));
  sinkApps.Start (Seconds (start_time_sink));
  sinkApps.Stop (Seconds (run_time));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (source), UdpSocketFactory::GetTypeId ()); 		//Creating udp socket
  ns3UdpSocket->SetAttribute("RcvBufSize",  ns3::UintegerValue(bsize));

  Ptr<MyApp> udp_Agent = CreateObject<MyApp> ();		//Creating sender application
  udp_Agent->Setup (ns3UdpSocket, sinkAddress, packetSize, packetsToSend, DataRate ("20Mbps"));
  nodes.Get (source)->AddApplication (udp_Agent);
  udp_Agent->SetStartTime (Seconds (start_time_apps));
  udp_Agent->SetStopTime (Seconds (run_time));

  return udp_Agent;

}

void SetPlot(Gnuplot &plot,std :: string plotTitle, std :: string x, std :: string y, std :: string extra, Gnuplot2dDataset &dataset);
void GeneratePlotFile(std :: string filename,Gnuplot &plot);
void CreateGraph(std :: string filename, std :: string plotTitle, std :: string x, std :: string y, std :: string extra, Gnuplot2dDataset &dataset);


int main(int argc, char *argv[])
{
	
	Time::SetResolution(Time::NS);

	//Default configuration is set as TCP New Reno
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));

	uint32_t bufferSize;
	dataset.SetTitle("Fairness");
	dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp.SetTitle("TCP Throughput");
	datasetTcp.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetUdp.SetTitle("UDP Throughput");
	datasetUdp.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp_udprate.SetTitle("TCP Throughput");
	datasetTcp_udprate.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetUdp_udprate.SetTitle("UDP Throughput");
	datasetUdp_udprate.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetOtherUdp_udprate.SetTitle("UDP Throughput of Remaining   UDP ");
	datasetOtherUdp_udprate.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp1.SetTitle("TCP Throughput of Connection 1");
	datasetTcp1.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp2.SetTitle("TCP Throughput of Connection 2");
	datasetTcp2.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp3.SetTitle("TCP Throughput of Connection 3");
	datasetTcp3.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetTcp4.SetTitle("TCP Throughput of Connection 4");
	datasetTcp4.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetUdp1.SetTitle("UDP Throughput of Connection 1");
	datasetUdp1.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetUdp2.SetTitle("UDP Throughput of Connection 2");
	datasetUdp2.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	int firstTime = 1;
	for (bufferSize = 10 * packetSize; bufferSize < 260* packetSize;bufferSize+=25*packetSize)
	{
		//Creating 8 nodes
		NodeContainer nodes;
		nodes.Create(8);

		//Create node containers for every link
		NodeContainer n0_3 = NodeContainer(nodes.Get(0), nodes.Get(3)); //H1R1
		NodeContainer n1_3 = NodeContainer(nodes.Get(1), nodes.Get(3)); //H2R1
		NodeContainer n2_3 = NodeContainer(nodes.Get(2), nodes.Get(3)); //H3R1
		NodeContainer n3_4 = NodeContainer(nodes.Get(3), nodes.Get(4)); //R1R2
		NodeContainer n4_5 = NodeContainer(nodes.Get(4), nodes.Get(5)); //R2H4
		NodeContainer n4_6 = NodeContainer(nodes.Get(4), nodes.Get(6)); //R2H5
		NodeContainer n4_7 = NodeContainer(nodes.Get(4), nodes.Get(7)); //R2H6

		//Install the internet stack(protocols) on the nodes
		InternetStackHelper internet;
		internet.Install(nodes);

		//Create point to point channels between the nodes
		PointToPointHelper p2p;

		//Host to router links
		p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p.SetChannelAttribute("Delay", StringValue("10ms"));
		NetDeviceContainer d0_3 = p2p.Install(n0_3);
		NetDeviceContainer d1_3 = p2p.Install(n1_3);
		NetDeviceContainer d2_3 = p2p.Install(n2_3);
		NetDeviceContainer d4_5 = p2p.Install(n4_5);
		NetDeviceContainer d4_6 = p2p.Install(n4_6);
		NetDeviceContainer d4_7 = p2p.Install(n4_7);

		//Router to router links
		//Setting the queueing configuration in the router to router link to DropTailQueue as mentioned in the question.
		p2p.SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("85p")));
		p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		p2p.SetChannelAttribute("Delay", StringValue("100ms"));
		NetDeviceContainer d3_4 = p2p.Install(n3_4);

		//Assign IP addresses to every interface
		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("7.7.1.0", "255.255.255.0");
		Ipv4InterfaceContainer i0_3 = ipv4.Assign (d0_3);
		ipv4.SetBase ("7.7.2.0", "255.255.255.0");
		Ipv4InterfaceContainer i1_3 = ipv4.Assign (d1_3);
		ipv4.SetBase ("7.7.3.0", "255.255.255.0");
		Ipv4InterfaceContainer i2_3 = ipv4.Assign (d2_3);
		ipv4.SetBase ("7.7.4.0", "255.255.255.0");
		Ipv4InterfaceContainer i3_4 = ipv4.Assign (d3_4);
		ipv4.SetBase ("7.7.5.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_5 = ipv4.Assign (d4_5);
		ipv4.SetBase ("7.7.6.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_6 = ipv4.Assign (d4_6);
		ipv4.SetBase ("7.7.7.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_7 = ipv4.Assign (d4_7);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	
		//TCP Reno Connections
		establishTCP(nodes, i4_6, 5, 6, bufferSize);		//h4 to h5
		establishTCP(nodes, i4_7, 0, 7, bufferSize);		//h1 to h6
		establishTCP(nodes, i4_7, 5, 7, bufferSize);		//h4 to h6
		establishTCP(nodes, i4_6, 0, 6, bufferSize);		//h1 to h5

		//CBR traffic on UDP
		FlowMonitorHelper flowMon;
		Ptr<FlowMonitor> flowmon_ptr = flowMon.InstallAll();
		establishUDP(nodes, i0_3, 5, 0, bufferSize);								//h4 to h1
		Ptr<MyApp> udp_to_change = establishUDP(nodes, i2_3, 1, 2, bufferSize);		//h2 to h3

		if(firstTime){
			Simulator::Schedule(Seconds(2.0), &IncRate, udp_to_change, DataRate("30Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(3.0), &IncRate, udp_to_change, DataRate("40Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(4.0), &IncRate, udp_to_change, DataRate("50Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(5.0), &IncRate, udp_to_change, DataRate("60Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(6.0), &IncRate, udp_to_change, DataRate("70Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(7.0), &IncRate, udp_to_change, DataRate("80Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(8.0), &IncRate, udp_to_change, DataRate("90Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(9.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 1);
			Simulator::Schedule(Seconds(10.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 1);
		}else{
			Simulator::Schedule(Seconds(2.0), &IncRate, udp_to_change, DataRate("30Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(3.0), &IncRate, udp_to_change, DataRate("40Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(4.0), &IncRate, udp_to_change, DataRate("50Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(5.0), &IncRate, udp_to_change, DataRate("60Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(6.0), &IncRate, udp_to_change, DataRate("70Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(7.0), &IncRate, udp_to_change, DataRate("80Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(8.0), &IncRate, udp_to_change, DataRate("90Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(9.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 0);
			Simulator::Schedule(Seconds(10.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 0);
		}
		

		firstTime=0;
		NS_LOG_INFO("Run Simulation.");
		Simulator::Stop(Seconds(10.0));
		Simulator::Run();

		flowmon_ptr->CheckForLostPackets();
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMon.GetClassifier());
		std::map<FlowId, FlowMonitor::FlowStats> fmStats = flowmon_ptr->GetFlowStats();
		double totFlow = 0, totFlowSquare = 0;
		double TCPsum = 0, UDPsum = 0;
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = fmStats.begin(); i != fmStats.end(); ++i)
		{
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
			double throughPut = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / (1024 * 1024);
			totFlow += throughPut;
			totFlowSquare += throughPut * throughPut;
			if (t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.6.2")
			{
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.7.2")
			{
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.7.2")
			{
				TCPsum += throughPut;
			}
			else if(t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.6.2"){
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.1.1")
			{
				UDPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.2.1" && t.destinationAddress == "7.7.3.1")
			{
				UDPsum += throughPut;
			}
		}
		double FairnessIndex = (totFlow * totFlow) / (6 * totFlowSquare);
		int NoOfPackets=bufferSize / packetSize;
		dataset.Add(NoOfPackets, FairnessIndex);
		datasetTcp.Add(NoOfPackets, TCPsum);
		datasetUdp.Add(NoOfPackets, UDPsum);

		std ::cout << "Buffer Size: " << NoOfPackets << "packets,\t\t";
		std ::cout << "Fairness Index: " << FairnessIndex << ",\t\t";
		std ::cout << "TCP Throughput: " << TCPsum << ",\t\t";
		std ::cout << "UDP Throughput: " << UDPsum << std ::endl;

		Simulator::Destroy();
	}
	for (; bufferSize <= 800* packetSize;bufferSize+=90*packetSize)
	{
		NodeContainer nodes;
		nodes.Create(8);

		NodeContainer n0_3 = NodeContainer(nodes.Get(0), nodes.Get(3)); //H1R1
		NodeContainer n1_3 = NodeContainer(nodes.Get(1), nodes.Get(3)); //H2R1
		NodeContainer n2_3 = NodeContainer(nodes.Get(2), nodes.Get(3)); //H3R1
		NodeContainer n3_4 = NodeContainer(nodes.Get(3), nodes.Get(4)); //R1R2
		NodeContainer n4_5 = NodeContainer(nodes.Get(4), nodes.Get(5)); //R2H4
		NodeContainer n4_6 = NodeContainer(nodes.Get(4), nodes.Get(6)); //R2H5
		NodeContainer n4_7 = NodeContainer(nodes.Get(4), nodes.Get(7)); //R2H6
	
		InternetStackHelper internet;
		internet.Install(nodes);

		PointToPointHelper p2p;
		//Host to router links
		p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p.SetChannelAttribute("Delay", StringValue("10ms"));
		NetDeviceContainer d0_3 = p2p.Install(n0_3);
		NetDeviceContainer d1_3 = p2p.Install(n1_3);
		NetDeviceContainer d2_3 = p2p.Install(n2_3);
		NetDeviceContainer d4_5 = p2p.Install(n4_5);
		NetDeviceContainer d4_6 = p2p.Install(n4_6);
		NetDeviceContainer d4_7 = p2p.Install(n4_7);

		//Router to router links
		//Setting the queueing configuration in the router to router link to DropTailQueue as mentioned in the question.
		p2p.SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("85p")));
		p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		p2p.SetChannelAttribute("Delay", StringValue("100ms"));
		NetDeviceContainer d3_4 = p2p.Install(n3_4);

		//Assign IP addresses to every interface
		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("7.7.1.0", "255.255.255.0");
		Ipv4InterfaceContainer i0_3 = ipv4.Assign (d0_3);
		ipv4.SetBase ("7.7.2.0", "255.255.255.0");
		Ipv4InterfaceContainer i1_3 = ipv4.Assign (d1_3);
		ipv4.SetBase ("7.7.3.0", "255.255.255.0");
		Ipv4InterfaceContainer i2_3 = ipv4.Assign (d2_3);
		ipv4.SetBase ("7.7.4.0", "255.255.255.0");
		Ipv4InterfaceContainer i3_4 = ipv4.Assign (d3_4);
		ipv4.SetBase ("7.7.5.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_5 = ipv4.Assign (d4_5);
		ipv4.SetBase ("7.7.6.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_6 = ipv4.Assign (d4_6);
		ipv4.SetBase ("7.7.7.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_7 = ipv4.Assign (d4_7);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	
		//TCP Reno Connections
		establishTCP(nodes, i4_6, 5, 6, bufferSize);		//h4 to h5
		establishTCP(nodes, i4_7, 0, 7, bufferSize);		//h1 to h6
		establishTCP(nodes, i4_7, 5, 7, bufferSize);		//h4 to h6
		establishTCP(nodes, i4_6, 0, 6, bufferSize);		//h1 to h5

		//CBR traffic on UDP
		FlowMonitorHelper flowMon;
		Ptr<FlowMonitor> flowmon_ptr = flowMon.InstallAll();
		establishUDP(nodes, i0_3, 5, 0, bufferSize);								//h4 to h1
		Ptr<MyApp> udp_to_change = establishUDP(nodes, i2_3, 1, 2, bufferSize);		//h2 to h3

		//increasing the UDP data rate upto 100Mbps in discrete time steps for UDP between h2 and h3
		Simulator::Schedule(Seconds(2.0), &IncRate, udp_to_change, DataRate("30Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(3.0), &IncRate, udp_to_change, DataRate("40Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(4.0), &IncRate, udp_to_change, DataRate("50Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(5.0), &IncRate, udp_to_change, DataRate("60Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(6.0), &IncRate, udp_to_change, DataRate("70Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(7.0), &IncRate, udp_to_change, DataRate("80Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(8.0), &IncRate, udp_to_change, DataRate("90Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(9.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 0);
		Simulator::Schedule(Seconds(10.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowMon, flowmon_ptr, 0);
		NS_LOG_INFO("Run Simulation.");
		Simulator::Stop(Seconds(10.0));
		Simulator::Run();

		flowmon_ptr->CheckForLostPackets();
	
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMon.GetClassifier());
		std::map<FlowId, FlowMonitor::FlowStats> fmStats = flowmon_ptr->GetFlowStats();
		double totFlow = 0, totFlowSquare = 0;
		double TCPsum = 0, UDPsum = 0;
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = fmStats.begin(); i != fmStats.end(); ++i)
		{
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
			double throughPut = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / (1024 * 1024);
			totFlow += throughPut;
			totFlowSquare += throughPut * throughPut;
			if (t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.6.2")
			{
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.1.1" && t.destinationAddress == "7.7.7.2")
			{
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.7.2")
			{
				TCPsum += throughPut;
			}
			else if(t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.6.2"){
				TCPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.5.2" && t.destinationAddress == "7.7.1.1")
			{
				UDPsum += throughPut;
			}
			else if (t.sourceAddress == "7.7.2.1" && t.destinationAddress == "7.7.3.1")
			{
				UDPsum += throughPut;
			}
		}
		
		double FairnessIndex = (totFlow * totFlow) / (6 * totFlowSquare);
		int NoOfPackets=bufferSize / packetSize;
		dataset.Add(NoOfPackets, FairnessIndex);
		datasetTcp.Add(NoOfPackets, TCPsum);
		datasetUdp.Add(NoOfPackets, UDPsum);

		std ::cout << "Buffer Size: " << bufferSize / packetSize << "packets,\t\t";
		std ::cout << "Fairness Index: " << FairnessIndex << ",\t\t";
		std ::cout << "TCP Throughput: " << TCPsum << ",\t\t";
		std ::cout << "UDP Throughput: " << UDPsum << std ::endl;

		Simulator::Destroy();
	}
	
	

	//Creating The Graph Files

	CreateGraph("tcp_vs_udprate", "UDP Rate vs Throughput(TCP)", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetTcp_udprate);
	CreateGraph("udp_vs_udprate", "UDP Rate vs Throughput(UDP)", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetUdp_udprate);
	CreateGraph("otherudp1_vs_udprate", "UDP Rate vs Throughput of remaining   UDP connection", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetOtherUdp_udprate);
	CreateGraph("tcp1_vs_udprate", "UDP Rate vs TCP Throughput of Connection 1", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetTcp1);
	CreateGraph("tcp2_vs_udprate", "UDP Rate vs TCP Throughput of Connection 2", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetTcp2);
	CreateGraph("tcp3_vs_udprate", "UDP Rate vs TCP Throughput of Connection 3", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetTcp3);
	CreateGraph("tcp4_vs_udprate", "UDP Rate vs TCP Throughput of Connection 4", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetTcp4);
	CreateGraph("udp1_vs_udprate", "UDP Rate vs UDP Throughput of Connection 1", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetUdp1);
	CreateGraph("udp2_vs_udprate", "UDP Rate vs UDP Throughput of Connection 2", "UDP Rate(Mbps)", "Throughput(Mbps)", "set xrange [10:100]", datasetUdp2);
	CreateGraph("fairness_vs_buffersize", "Buffer Size vs Fairness plot", "Buffer Size(# of packets)", "Fairness", "set xrange [0:800]", dataset);
	CreateGraph("tcp_vs_buffersize", "Buffer Size vs Throughput(TCP)", "Buffer Size(# of packets)", "Throughput(Mbps)", "set xrange [0:800]", datasetTcp);
	CreateGraph("udp_vs_buffersize", "Buffer Size vs Throughput(UDP)", "Buffer Size(# of packets)", "Throughput(Mbps)", "set xrange [0:800]", datasetUdp);
	NS_LOG_INFO("Finished.");
}

void SetPlot(Gnuplot &plot,std :: string plotTitle, std :: string x, std :: string y, std :: string extra, Gnuplot2dDataset &dataset)
{
  plot.SetTitle (plotTitle);
	plot.SetTerminal ("png");
	plot.SetLegend (x,y);
	plot.AppendExtra (extra);
	plot.AddDataset (dataset);
}

void GeneratePlotFile(std :: string filename,Gnuplot &plot)
{
  std::string pltfname = filename + ".plt";
	std::ofstream plotFile (pltfname.c_str());
	plot.GenerateOutput (plotFile);
	plotFile.close ();
}

void CreateGraph(std :: string filename, std :: string plotTitle, std :: string x, std :: string y, std :: string extra, Gnuplot2dDataset &dataset)
{
	std :: string pngfname = filename + ".png";
	Gnuplot plot (pngfname);
    SetPlot(plot,plotTitle,x,y,extra,dataset);
    GeneratePlotFile(filename,plot);
}
