#include "ClockPublisherItem.h"
#include <QCoreApplication>
#include <cnoid/ItemManager>
#include <rosgraph_msgs/Clock.h>

namespace cnoid {

  void ClockPublisherItem::initializeClass(ExtensionManager* ext)
  {
    ext->itemManager().registerClass<ClockPublisherItem>("ClockPublisherItem");
  }

  ClockPublisherItem::ClockPublisherItem(){
    if(!ros::isInitialized()){
      QStringList argv_list = QCoreApplication::arguments();
      int argc = argv_list.size();
      char* argv[argc];
      //なぜかわからないがargv_list.at(i).toUtf8().data()のポインタをそのままargvに入れるとros::initがうまく解釈してくれない.
      for(size_t i=0;i<argv_list.size();i++){
        char* data = argv_list.at(i).toUtf8().data();
        size_t dataSize = 0;
        for(size_t j=0;;j++){
          if(data[j] == '\0'){
            dataSize = j;
            break;
          }
        }
        argv[i] = (char *)malloc(sizeof(char) * dataSize+1);
        for(size_t j=0;j<dataSize;j++){
          argv[i][j] = data[j];
        }
        argv[i][dataSize] = '\0';
      }
      ros::init(argc,argv,"choreonoid");
      for(size_t i=0;i<argc;i++){
        free(argv[i]);
      }
    }
    this->clockPublisher_ = ros::NodeHandle().advertise<rosgraph_msgs::Clock>("/clock", 1);
    ros::NodeHandle().setParam("/use_sim_time", true);
    SimulationBar::instance()->sigSimulationAboutToStart().connect([&](SimulatorItem* simulatorItem){onSimulationAboutToStart(simulatorItem);});
  }

  void ClockPublisherItem::onSimulationAboutToStart(SimulatorItem* simulatorItem)
  {
    this->currentSimulatorItem_ = simulatorItem;
    this->currentSimulatorItemConnections_.add(
        simulatorItem->sigSimulationStarted().connect(
            [&](){ onSimulationStarted(); }));
  }

  void ClockPublisherItem::onSimulationStarted()
  {
    this->currentSimulatorItem_->addMidDynamicsFunction([&](){ onSimulationStep(); });
  }

  void ClockPublisherItem::onSimulationStep()
  {
    double time = this->currentSimulatorItem_->simulationTime();

    // Publish clock
    rosgraph_msgs::Clock clock;
    clock.clock.fromSec(time);
    this->clockPublisher_.publish(clock);
  }
}

