// -*- C++ -*-
// $Id$

/**
 * Code generated by the The ACE ORB (TAO) IDL Compiler v2.1.9
 * TAO and the TAO IDL Compiler have been developed by:
 *       Center for Distributed Object Computing
 *       Washington University
 *       St. Louis, MO
 *       USA
 *       http://www.cs.wustl.edu/~schmidt/doc-center.html
 * and
 *       Distributed Object Computing Laboratory
 *       University of California at Irvine
 *       Irvine, CA
 *       USA
 * and
 *       Institute for Software Integrated Systems
 *       Vanderbilt University
 *       Nashville, TN
 *       USA
 *       http://www.isis.vanderbilt.edu/
 *
 * Information about TAO is available at:
 *     http://www.cs.wustl.edu/~schmidt/TAO.html
 **/


// TAO_IDL - Generated from
// c:\users\toshi\src\vc11\ace_wrappers\tao\tao_idl\be\be_visitor_union\union_ci.cpp:39

// *************************************************************
// Inline operations for union TOFSignal::IonSourceSetpts
// *************************************************************

// TAO_IDL - Generated from
// c:\users\toshi\src\vc11\ace_wrappers\tao\tao_idl\be\be_visitor_union\discriminant_ci.cpp:58

ACE_INLINE
void
TOFSignal::IonSourceSetpts::_default ()
{
  this->_reset ();
  this->disc_ = TOF_C::eIonSource_Unknown;
}

// Accessor to set the discriminant.
ACE_INLINE
void
TOFSignal::IonSourceSetpts::_d (TOF_C::eIonSource discval)
{
  this->disc_ = discval;
}

// Accessor to get the discriminant.
ACE_INLINE
TOF_C::eIonSource
TOFSignal::IonSourceSetpts::_d (void) const
{
  return this->disc_;
}

// TAO_IDL - Generated from
// c:\users\toshi\src\vc11\ace_wrappers\tao\tao_idl\be\be_visitor_union_branch\public_ci.cpp:1068

/// Accessor to set the member.
ACE_INLINE
void
TOFSignal::IonSourceSetpts::ei (const TOFSignal::IonSource_EI_Setpts &val)
{
  // Set the discriminant value.
  this->_reset ();
  this->disc_ = TOF_C::eIonSource_EI;
  this->u_.ei_ = val;
}

// Readonly get method.
ACE_INLINE
const TOFSignal::IonSource_EI_Setpts &
TOFSignal::IonSourceSetpts::ei (void) const
{
  return this->u_.ei_;
}

// Read/write get method.
ACE_INLINE
TOFSignal::IonSource_EI_Setpts &
TOFSignal::IonSourceSetpts::ei (void)
{
  return this->u_.ei_;
}

// TAO_IDL - Generated from
// c:\users\toshi\src\vc11\ace_wrappers\tao\tao_idl\be\be_visitor_union_branch\public_ci.cpp:1068

/// Accessor to set the member.
ACE_INLINE
void
TOFSignal::IonSourceSetpts::dart (const TOFSignal::IonSource_DART_Setpts &val)
{
  // Set the discriminant value.
  this->_reset ();
  this->disc_ = TOF_C::eIonSource_DART;
  this->u_.dart_ = val;
}

// Readonly get method.
ACE_INLINE
const TOFSignal::IonSource_DART_Setpts &
TOFSignal::IonSourceSetpts::dart (void) const
{
  return this->u_.dart_;
}

// Read/write get method.
ACE_INLINE
TOFSignal::IonSource_DART_Setpts &
TOFSignal::IonSourceSetpts::dart (void)
{
  return this->u_.dart_;
}

// TAO_IDL - Generated from
// c:\users\toshi\src\vc11\ace_wrappers\tao\tao_idl\be\be_visitor_interface\interface_ci.cpp:62

ACE_INLINE
TOFSignal::no_empty_file::no_empty_file (
    TAO_Stub *objref,
    ::CORBA::Boolean _tao_collocated,
    TAO_Abstract_ServantBase *servant,
    TAO_ORB_Core *oc)
  : ::CORBA::Object (objref, _tao_collocated, servant, oc)
{
}

ACE_INLINE
TOFSignal::no_empty_file::no_empty_file (
    ::IOP::IOR *ior,
    TAO_ORB_Core *oc)
  : ::CORBA::Object (ior, oc)
{
}

