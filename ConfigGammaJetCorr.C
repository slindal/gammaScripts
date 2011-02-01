void ConfigGammaJetCorr (TString inputfile = "files.txt" ) {

  gSystem->Load("libTree.so");
  gSystem->Load("libPhysics.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("libVMC.so");
  gSystem->Load("libSTEERBase.so");
  gSystem->Load("libESD.so");
  gSystem->Load("libAOD.so"); 
  gSystem->Load("libANALYSIS");
  gSystem->Load("libANALYSISalice");
  gSystem->Load("libJETAN.so");
  gSystem->Load("libPWG4GammaConv.so");
  gSystem->Load("libPWG4PartCorrBase.so");
  gSystem->Load("libPWG4PartCorrDep.so");
  gSystem->AddIncludePath("-I$ALICE_ROOT/include");


  TChain * chain = createAODChain (inputfile);
  if(!chain) {
    cout << "Errror in chain ceration"<<endl;
    return -1;
  }
    
  AliAnalysisManager *mgr  = new AliAnalysisManager("GammaJet Manager", "GammaJet Manager");
  mgr->SetDebugLevel(0);

  AliAODInputHandler * inpHandler = new AliAODInputHandler();
  
  //inpHandler->AddFriend("AliAODGammaConversion.root");
  //  inpHandler->SetDebugLevel(0);
  mgr->SetInputEventHandler(inpHandler);


  AliAnalysisTaskGCPartToPWG4Part * gammaJetAna = new AliAnalysisTaskGCPartToPWG4Part("gamma jet analysis");
  gammaJetAna->SetDebugLevel(0);
  //gammaJetAna->SetGammaBranchName("GammaConversionTask_900356200010031");
  gammaJetAna->SetDeltaAODFileName("");
  
  mgr->AddTask(gammaJetAna);
    
  AliAnalysisDataContainer *cinput1 = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput2 = mgr->CreateContainer("histos", TList::Class(), AliAnalysisManager::kOutputContainer, "histos.root");


  mgr->ConnectInput  (gammaJetAna,  0, cinput1  );
  mgr->ConnectOutput (gammaJetAna,  1, coutput2 );

  gROOT->LoadMacro("AddTaskPartCorr.C");
  AliAnalysisTaskParticleCorrelation *taskPartCorr = AddTaskPartCorr("AOD", "CTS",kTRUE,kTRUE);
  taskPartCorr->SetDebugLevel(0);
  //taskPartCorr->GetReader()->SetDataType(inputDataType::kMC);
  
  ///////////////////////////////AOD handler
  AliAODHandler* aodHandler = new AliAODHandler();
  aodHandler->SetOutputFileName("balle.root");
  aodHandler->SetCreateNonStandardAOD();
  mgr->SetOutputEventHandler(aodHandler);	
  //////////////////////////////Do globally

  
  mgr->InitAnalysis();
  mgr->PrintStatus();
  //mgr->StartAnalysis("local",chain, 1000);
  mgr->StartAnalysis("local",chain);

}

///________________________________________________________________________________________
void AddFriendBranches(TString inputfile = "") {

  TString line;
  ifstream in;
  in.open(inputfile.Data());
  while (in.good()) {
    in >> line;
    if (line.Length() == 0) continue;
    chain->AddFriend("aodTree", line.Data());
  }

  return chain;
}

///_________________________________________________________________________________________
TChain * createAODChain (TString inputfile = "files.txt") {

  chain = new TChain("aodTree");

  TString line;
  ifstream in;
  in.open(inputfile.Data());
  while (in.good()) {
    in >> line;
    if (line.Length() == 0) continue;
    chain->Add(line.Data());
  }

  return chain;
  
}
