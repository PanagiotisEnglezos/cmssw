#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "RecoTracker/TkTrackingRegions/interface/OrderedHitsGeneratorFactory.h"
#include "RecoTracker/TkTrackingRegions/interface/OrderedHitsGenerator.h"

#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducerFactory.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducer.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegion.h"
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"

#include "UserCode/konec/test/R2DTimerObserver.h"
#include "TH1D.h"
#include "TFile.h"



class HitTripletProducer : public edm::EDAnalyzer {
public:
  explicit HitTripletProducer(const edm::ParameterSet& conf);
  ~HitTripletProducer();
  virtual void beginJob(const edm::EventSetup& iSetup);
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() { }
private:
  edm::ParameterSet theConfig;
  OrderedHitsGenerator * theGenerator;
  TrackingRegionProducer* theRegionProducer;
  TH1D *hCPU, *hNum;
};

HitTripletProducer::HitTripletProducer(const edm::ParameterSet& conf) 
  : theConfig(conf), theGenerator(0)
{
  edm::LogInfo("HitTripletProducer")<<" CTOR";
  hCPU = new TH1D ("hCPU","hCPU",140,0.,0.070);
  hNum = new TH1D ("hNum","hNum",250,0.,500.);
}

HitTripletProducer::~HitTripletProducer() 
{ 
  edm::LogInfo("HitTripletProducer")<<" DTOR";
  delete theGenerator;

  TFile rootFile("analysis.root", "RECREATE", "my histograms");
  hCPU->Write();
  hNum->Write();
  rootFile.Close();
}

void HitTripletProducer::beginJob(const edm::EventSetup& es)
{
  edm::ParameterSet orderedPSet =
      theConfig.getParameter<edm::ParameterSet>("OrderedHitsFactoryPSet");
  std::string orderedName = orderedPSet.getParameter<std::string>("ComponentName");
  theGenerator = OrderedHitsGeneratorFactory::get()->create( orderedName, orderedPSet);

  edm::ParameterSet regfactoryPSet =
      theConfig.getParameter<edm::ParameterSet>("RegionFactoryPSet");
  std::string regfactoryName = regfactoryPSet.getParameter<std::string>("ComponentName");
  theRegionProducer = TrackingRegionProducerFactory::get()->create(regfactoryName,regfactoryPSet);

}

void HitTripletProducer::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{

//  GlobalTrackingRegion region;

  typedef std::vector<TrackingRegion* > Regions;
  Regions regions = theRegionProducer->regions(ev,es);
  const TrackingRegion & region = *regions[0];

  static R2DTimerObserver timer("**** MY TIMING REPORT ***");
  timer.start();
  const OrderedSeedingHits & triplets = theGenerator->run(region,ev,es);
  timer.stop(); 
  hCPU->Fill( timer.lastMeasurement().real() );
  hNum->Fill(triplets.size());
  edm::LogInfo("HitTripletProducer") << "size of triplets: "<<triplets.size();

}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(HitTripletProducer);
