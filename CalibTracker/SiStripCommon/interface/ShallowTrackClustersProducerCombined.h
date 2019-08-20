#ifndef SHALLOW_TRACKCLUSTERS_PRODUCER_COMBINED
#define SHALLOW_TRACKCLUSTERS_PRODUCER_COMBINED

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
class SiStripClusterInfo;
class SiStripProcessedRawDigi;
class TrackerTopology;
class SiStripLorentzAngle;

class ShallowTrackClustersProducerCombined : public edm::EDProducer {
public:
  explicit ShallowTrackClustersProducerCombined(const edm::ParameterSet&);
private:
  const edm::EDGetTokenT<edm::View<reco::Track> > tracks_token_;
  const edm::EDGetTokenT<TrajTrackAssociationCollection> association_token_;
  const edm::EDGetTokenT< edmNew::DetSetVector<SiStripCluster> > clusters_token_;
  edm::EDGetTokenT<std::vector<reco::Vertex> >          theVertexToken_;
  edm::EDGetTokenT<edm::DetSetVector<SiStripProcessedRawDigi> > theDigisToken_;
  std::string Suffix;
  std::string Prefix;
  int32_t lowBound;
  int32_t highBound;
  std::string filename;
  edm::ESHandle<SiStripLorentzAngle> lorentzAngleHandle;
  const std::string lorentzAngleName;
  bool isData;

  void produce( edm::Event &, const edm::EventSetup & );
  bool trackFilter(const reco::Track* trk);

  struct moduleVars {
    moduleVars(uint32_t, const TrackerTopology*);
    int subdetid, side, layerwheel, stringringrod, petal, stereo;
    uint32_t module;
  };

  struct NearDigis { 
    NearDigis(const SiStripClusterInfo&);
    NearDigis(const SiStripClusterInfo&, const edm::DetSetVector<SiStripProcessedRawDigi>&);
    float max, left, right, first, last, Lleft, Rright; 
    float etaX() const {return ((left+right)/max)/2.;}
    float eta()  const {return right>left ? max/(max+right) : left/(left+max);}
    float etaasymm() const {return right>left ? (right-max)/(right+max) : (max-left)/(max+left);}
    float outsideasymm() const {return (last-first)/(last+first);}
  };
};
#endif
