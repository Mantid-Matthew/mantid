#ifndef MANTID_MDWS_SLICE_H_
#define MANTID_MDWS_SLICE_H_

#include <cxxtest/TestSuite.h>
#include "MantidMDEvents/MDWSTransfDescr.h"

using namespace Mantid;
using namespace Mantid::MDEvents;
using namespace Mantid::Kernel;




class MDWSTransfDescrTest : public CxxTest::TestSuite
{
    //MDWSSliceTest slice;
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MDWSTransfDescrTest *createSuite() { return new MDWSTransfDescrTest(); }
  static void destroySuite( MDWSTransfDescrTest *suite ) { delete suite; }


void test_buildDimNames(){

    MDEvents::MDWSDescription TargWSDescription(4);

    TargWSDescription.convert_to_factor=NoScaling;
    MDWSTransfDescr MsliceTransf;
 
    TS_ASSERT_THROWS_NOTHING(MsliceTransf.setQ3DDimensionsNames(TargWSDescription));


   TS_ASSERT_EQUALS("[Qh,0,0]",TargWSDescription.dimNames[0]);
   TS_ASSERT_EQUALS("[0,Qk,0]",TargWSDescription.dimNames[1]);
   TS_ASSERT_EQUALS("[0,0,Ql]",TargWSDescription.dimNames[2]);
    

}
void testTransfMat1()
{
     MDEvents::MDWSDescription TWS(4);
     TWS.pLatt = std::auto_ptr<Geometry::OrientedLattice>(new Geometry::OrientedLattice(10.4165,3.4165,10.4165, 90., 90., 90.));
     TWS.convert_to_factor=HKLScale;
     std::vector<double> u(3,0);
     std::vector<double> v(3,0);
     u[0]=1;
     v[2]=1;
     std::vector<double> rot;

      MDWSTransfDescr MsliceTransf;
    //  MsliceTransf.getUVsettings(u,v);     

      TS_ASSERT_THROWS_NOTHING(rot=MsliceTransf.getTransfMatrix("someDodgyWS",TWS,false));
      TS_ASSERT_THROWS_NOTHING(MsliceTransf.setQ3DDimensionsNames(TWS));


      TS_ASSERT_EQUALS("[Qh,0,0]",TWS.dimNames[0]);
      TS_ASSERT_EQUALS("[0,0,Ql]",TWS.dimNames[1]);
      TS_ASSERT_EQUALS("[0,-Qk,0]",TWS.dimNames[2]);


//     Kernel::Matrix<double> U0=TWS.pLatt->setUFromVectors(u,v);
//     std::vector<double> rot0=U0.getVector();
//     //ws2D->mutableSample().setOrientedLattice(latt);
//

////     TWS.is_uv_default=true;
//     TWS.emode=1;
//
//     // get transformation matrix from oriented lattice. 
//    // std::vector<double> rot=pAlg->getTransfMatrix("TestWS",TWS);
//  
//     for(int i=0;i<9;i++){
//        TS_ASSERT_DELTA(rot0[i],rot[i],1.e-6);
//     }
//     Kernel::Matrix<double> rez(rot);
//     V3D ez = rez*u;
//     ez.normalize();
//     V3D ex = rez*v;
//     ex.normalize();
//     TS_ASSERT_EQUALS(V3D(0,0,1),ez);
//     TS_ASSERT_EQUALS(V3D(1,0,0),ex);
//
//     // to allow recalculate axis names specific for Q3D mode
//     TWS.AlgID="WSEventQ3DPowdDirectCnvFromTOF";
// 
//
}



};
#endif