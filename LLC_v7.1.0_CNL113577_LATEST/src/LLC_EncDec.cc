///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2008                           //
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) has been supplied.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//  File:               LLC_EncDec.cc
//  Rev:                R1A0
//  Prodnr:             CNL 113 577
//  Updated:            2008-01-22
//  Contact:            http://ttcn.ericsson.se
//  Reference:          3GPP TS 44.064 7.1.0
 
#include "LLC_Types.hh"
#include <string.h>

// GIVEN IN CODE RECEIVED FROM ERV
#define CRC_POLY_LLC2  0xad85dd

// GIVEN IN SPECIFICATION
//#define CRC_POLY_LLC2  0xbba1b5

#define TABLE_LENGTH  256

// For UI frames if PM bit is 0 (unprotected) then CRC will be calculated over Header + N202 octets
#define N202 4

unsigned int mCRCTable[TABLE_LENGTH];


static void BuildCrc24Table()
{
 unsigned int i,j;
 unsigned int reg;

  //TRACE_FUNC( "BuildCrc24Table" );

  for( i = 0; i < TABLE_LENGTH; i++ )
  {
    reg = i;
    for( j = 8; j > 0; j-- )
    {
      if( reg & 1 )
      {
        
	reg = (reg>>1) ^ (unsigned int) CRC_POLY_LLC2;
      }
      else
      {
        reg >>= 1;
      }
    }
    reg &= 0x00ffffffL;
   
      mCRCTable[i] = (unsigned int)reg;
      //TTCN_Logger::begin_event(TTCN_DEBUG);
     
      //TTCN_Logger::log_event("mCRCTable[%d]= %d",i,mCRCTable[i]);
  
     // TTCN_Logger::end_event();

  }
}

//---------------------------------------------------------------------------------------

unsigned int  Calculate_CRC(TTCN_Buffer& pdu)
{

const unsigned char* loc_pdu = pdu.get_data();

 //TTCN_Logger::begin_event(TTCN_DEBUG);
     
     //for (size_t i =0; i< pdu.get_len();i++)
    // {
     // TTCN_Logger::log_event("%02x ",loc_pdu[i]);
    // }
     
//TTCN_Logger::log_event("\n");      
     
//TTCN_Logger::end_event();

BuildCrc24Table();

//TTCN_Logger::begin_event(TTCN_DEBUG);
//TTCN_Logger::log_event("mCRCTable[%d]= %d\n",255,mCRCTable[255]);
//TTCN_Logger::log_event("mCRCTable[%d]= 0x%08x (%u)",255,mCRCTable[255]);
//TTCN_Logger::log_event("\n");      
//TTCN_Logger::end_event();

unsigned int reg = 0xFFFFFF;

unsigned int length = pdu.get_len();


      
if (  ((loc_pdu[1] >>5) & 0x07) == 0x06 )      //UI frame
{ 
 if ((loc_pdu[2] & 0x01) == 0x00)              // PM bit is 0 (unprotected)         
  { 
     if(length  > 3 + N202)                     // pdu length is longer than header + N202    
     {
     length = 3 + N202;                         // length = header length + N202    
          
     }	
   }              
}

   

while ( length--)
  {
    reg =
      ((reg>>8) & 0x00ffff)^mCRCTable[(reg^*((unsigned char*)loc_pdu++)) & 0xffL];
  }
   //TTCN_Logger::log(TTCN_DEBUG, "reg:%08x\n",reg);	
   // 1's complement of FCS 
  reg ^= 0xffffffL;
  
  reg = ((reg >> 16) & 0x000000ff)+ ((reg)  & 0x0000ff00) + ((reg <<16 )& 0x00ff0000);
 //unsigned int tempint = *(unsigned int*)loc_crcBuffer;
 //reg =  (unsigned int*)loc_crcBuffer ;
 //TTCN_Logger::log(TTCN_DEBUG, "reg:%08x\n",reg);	
	
return  reg & 0x00ffffffL;	
}
//---------------------------------------------------------------------------------------

namespace LLC__Types {

OCTETSTRING enc__PDU__LLC(const PDU__LLC& pdu)
{
	TTCN_Buffer bb;
	PDU__LLC pdu2(pdu);
	
	if (pdu2.get_selection() == PDU__LLC::ALT_pDU__LLC__U)
	{

          if (pdu2.pDU__LLC__U().information__field__U().get_selection() ==  Information__field__U::ALT_uA)
           {int record_of_size = pdu2.pDU__LLC__U().information__field__U().uA().size_of();
       
            for (int  i = 0; i < (record_of_size) ; i++)
	      { 
	      // AUTOMATICALLY CALCULATE WHICH LENGTH FORMAT SHOULD BE USED AND CHANGE SHORT LENGTH FORM 
	      // TO LONG LENGTH FORM IF NECESSARY WHEN L3 PDU IS INCLUDED
	         if ( pdu2.pDU__LLC__U().information__field__U().uA()[i].typefield() == int2bit(11,5) )
	          {  
		   if( pdu2.pDU__LLC__U().information__field__U().uA()[i].xID__Data().l3param().lengthof() > 3)
		   {		  
		   pdu2.pDU__LLC__U().information__field__U().uA()[i].xID__length().long__len() = 
		   pdu2.pDU__LLC__U().information__field__U().uA()[i].xID__Data().l3param().lengthof();		   
		   }		 		 		  
		  }
	      } 
           }

           if (pdu2.pDU__LLC__U().information__field__U().get_selection() ==  Information__field__U::ALT_sABM)
        {int record_of_size = pdu2.pDU__LLC__U().information__field__U().sABM().size_of();
       
            for (int  i = 0; i < (record_of_size) ; i++)
	      { 
	      // AUTOMATICALLY CALCULATE WHICH LENGTH FORMAT SHOULD BE USED AND CHANGE SHORT LENGTH FORM 
	      // TO LONG LENGTH FORM IF NECESSARY WHEN L3 PDU IS INCLUDED
	         if ( pdu2.pDU__LLC__U().information__field__U().sABM()[i].typefield() == int2bit(11,5) )
	          {  
		   if( pdu2.pDU__LLC__U().information__field__U().sABM()[i].xID__Data().l3param().lengthof() > 3)
		   {		  
		   pdu2.pDU__LLC__U().information__field__U().sABM()[i].xID__length().long__len() = 
		   pdu2.pDU__LLC__U().information__field__U().sABM()[i].xID__Data().l3param().lengthof();		   
		   }		 		 		  
		  }
	      } 
           }
		
            if (pdu2.pDU__LLC__U().information__field__U().get_selection() ==  Information__field__U::ALT_xID)
       {int record_of_size = pdu2.pDU__LLC__U().information__field__U().xID().size_of();
       
            for (int  i = 0; i < (record_of_size) ; i++)
	      { 
	      // AUTOMATICALLY CALCULATE WHICH LENGTH FORMAT SHOULD BE USED AND CHANGE SHORT LENGTH FORM 
	      // TO LONG LENGTH FORM IF NECESSARY WHEN L3 PDU IS INCLUDED
	         if ( pdu2.pDU__LLC__U().information__field__U().xID()[i].typefield() == int2bit(11,5) )
	          {  
		   if( pdu2.pDU__LLC__U().information__field__U().xID()[i].xID__Data().l3param().lengthof() > 3)
		   {		  
		   pdu2.pDU__LLC__U().information__field__U().xID()[i].xID__length().long__len() = 
		   pdu2.pDU__LLC__U().information__field__U().xID()[i].xID__Data().l3param().lengthof();		   
		   }		 		 		  
		  }
	      } 
           }			
	}
	
	
	if (pdu2.get_selection() == PDU__LLC::ALT_pDU__LLC__U)
	 {
	    if ( pdu2.pDU__LLC__U().fCS().ispresent())
	    {
	        if ( pdu2.pDU__LLC__U().fCS() == int2oct(0,3) )  // IF ENCODER NEEDS TO GENERATE CRC
	          {      
                    pdu2.pDU__LLC__U().fCS() = OMIT_VALUE;
	            pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);
	            unsigned int crcBuffer = Calculate_CRC(bb);
		    bb.put_os(int2oct(crcBuffer,3));		
	            return OCTETSTRING (bb.get_len(), bb.get_data());
	
		  }
	    
	     else {	// IF ENCODER SENDS OUT NONZERO CRC GIVEN IN TTCN TEMPLATE
	         pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);	
	         return OCTETSTRING (bb.get_len(), bb.get_data());
	          }    	    
	    }	
	    else {	//FCS OMIT
	            pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);
	            unsigned int crcBuffer = Calculate_CRC(bb);
		    bb.put_os(int2oct(crcBuffer,3));		
	            return OCTETSTRING (bb.get_len(), bb.get_data());
		    
	         }
	 }
	
	
	
	else if (pdu2.get_selection() == PDU__LLC::ALT_pDU__LLC__UI)
	    {
	      if ( pdu2.pDU__LLC__UI().fCS().ispresent())
	       {
	          if ( pdu2.pDU__LLC__UI().fCS() == int2oct(0,3) )   // IF ENCODER NEEDS TO GENERATE CRC 
	          {      
		    pdu2.pDU__LLC__UI().fCS() = OMIT_VALUE;
	            pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);
	            unsigned int crcBuffer = Calculate_CRC(bb);
		    bb.put_os(int2oct(crcBuffer,3));		
	            return OCTETSTRING (bb.get_len(), bb.get_data()); 
		  
		  }
		  
	     else {	// IF ENCODER SENDS OUT NONZERO CRC GIVEN IN TTCN TEMPLATE
	          pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW); 	
	          return OCTETSTRING (bb.get_len(), bb.get_data());	    
	          }
	       }
	     else {	//FCS OMIT
	            pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);
	            unsigned int crcBuffer = Calculate_CRC(bb);
		    bb.put_os(int2oct(crcBuffer,3));		
	            return OCTETSTRING (bb.get_len(), bb.get_data()); 
	          }		
	    }
	else {TTCN_error("Can not encode LLC PDU");  //Neither UI NOR U
	
	      return  OCTETSTRING (bb.get_len(), bb.get_data()); // this is dummy to avoid warning during compilation
	    }
	//pdu2.encode(PDU__LLC_descr_ ,bb, TTCN_EncDec::CT_RAW);	
	//unsigned int crcBuffer = Calculate_CRC(bb);
		
	//bb.put_os( int2oct(crcBuffer,3));
	
	
	//return OCTETSTRING (bb.get_len(), bb.get_data());
		
}


OCTETSTRING enc__PDU__LLC(const PDU__LLC_template& pdu)
{
  return enc__PDU__LLC(pdu.valueof());
}


PDU__LLC dec__PDU__LLC(const OCTETSTRING& stream)
{
	PDU__LLC retv;
	TTCN_Buffer bb;
        OCTETSTRING crc = int2oct(0,3);
	
	size_t datalength = stream.lengthof()-3;
	const unsigned char * CRC_AS_RECEIVED = (const unsigned char *)stream+datalength;
	bb.put_s(datalength,(const unsigned char *)stream);
	
	unsigned int CRC_CALCULATED = Calculate_CRC(bb);
     
      // COMPARE CRC RECEIVED IN LLC PDU WITH CRC CALCULATED FROM LLC PDU	
	if(
	  (CRC_AS_RECEIVED[ 0 ] != (CRC_CALCULATED & 0xff0000  ) >> 16) ||
          (CRC_AS_RECEIVED[ 1 ] != (CRC_CALCULATED & 0xff00    ) >> 8) ||
          (CRC_AS_RECEIVED[ 2 ] != (CRC_CALCULATED & 0xff      )     )
	  ) 
	{
	     TTCN_warning("CRC ERROR IN LLC PDU");  // CRC IS NOT AS EXPECTED
	     crc=OCTETSTRING(3,CRC_AS_RECEIVED);
	}
	 // CRC IS 
         // FILL CRC octets with zeroes if CRC is OK
	
	retv.decode(PDU__LLC_descr_, bb, TTCN_EncDec::CT_RAW);
	 
	if (retv.get_selection() == PDU__LLC::ALT_pDU__LLC__UI){
	   retv.pDU__LLC__UI().fCS() = crc;
	} 
	    
	if (retv.get_selection() == PDU__LLC::ALT_pDU__LLC__U){
	retv.pDU__LLC__U().fCS() = crc;
	} 
	
	
	return retv;
	   

}

}//namespace
