// -*- C++ -*-

//=============================================================================
/**
 *  @file     IIOP_Endpoint.h
 *
 *  IIOP implementation of PP Framework Endpoint interface.
 *
 *  @author  Marina Spivak <marina@cs.wustl.edu>
 */
//=============================================================================

#ifndef TAO_IIOP_ENDPOINT_H
#define TAO_IIOP_ENDPOINT_H

#include /**/ "ace/pre.h"

#include "tao/orbconf.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#if defined (TAO_HAS_IIOP) && (TAO_HAS_IIOP != 0)

#include "tao/CORBA_String.h"
#include "tao/IIOP_EndpointsC.h"
#include "tao/Endpoint.h"

#include "ace/INET_Addr.h"
#include "ace/Vector_T.h"

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

class TAO_IIOP_Connection_Handler;
class TAO_IIOP_Profile;

/**
 * @class TAO_IIOP_Endpoint
 *
 * @brief TAO_IIOP_Endpoint
 *
 * IIOP-specific implementation of PP Framework Endpoint interface.
 */
class TAO_Export TAO_IIOP_Endpoint : public TAO_Endpoint
{
public:
  // @todo Lose these friends!
  friend class TAO_IIOP_Profile;
  friend class TAO_SSLIOP_Profile;

  /// Default constructor.
  TAO_IIOP_Endpoint ();

  /// Constructor.  This is the most efficient constructor since it
  /// does not require any address resolution processing.
  TAO_IIOP_Endpoint (const char *host,
                     CORBA::UShort port,
                     const ACE_INET_Addr &addr,
                     CORBA::Short priority = TAO_INVALID_PRIORITY);

  /// Constructor.
  TAO_IIOP_Endpoint (const ACE_INET_Addr &addr,
                     int use_dotted_decimal_addresses);

  /// Constructor.  This constructor is used when decoding endpoints.
  TAO_IIOP_Endpoint (const char *host,
                     CORBA::UShort port,
                     CORBA::Short priority);

  /// Destructor.
  ~TAO_IIOP_Endpoint ();


  // = Implementation of abstract TAO_Endpoint methods.  See
  // Endpoint.h for their documentation.

  virtual TAO_Endpoint *next ();

  /**
   * Return the next endpoint in the list, but use protocol-specific
   * filtering to constrain the value. The orb core is needed to supply
   * any sort of filter arguments, and the root endpoint is needed in case
   * the algorithm needs to rewind. If the supplied root is 0, then this
   * is assumed to be the candidate next endpoint.
   *
   * To use this, the caller starts off the change with root == 0. This
   * is a bit of a violation in logic, a more correct implementation would
   * accept this == 0 and a non-null root.
   * To do iteration using next_filtered, do:
   *   for (TAO_Endpoint *ep = root_endpoint->next_filtered (orb_core, 0);
   *        ep != 0;
   *        ep = ep->next_filtered(orb_core, root_endpoint)) { }
   */
  virtual TAO_Endpoint *next_filtered (TAO_ORB_Core *, TAO_Endpoint *root);

  virtual int addr_to_string (char *buffer, size_t length);

  /// Makes a copy of @c this
  virtual TAO_Endpoint *duplicate ();

  /// Return true if this endpoint is equivalent to @a other_endpoint.  Two
  /// endpoints are equivalent if their port and host are the same.
  virtual CORBA::Boolean is_equivalent (const TAO_Endpoint *other_endpoint);

  /// Return a hash value for this object.
  virtual CORBA::ULong hash ();

  // = IIOP_Endpoint-specific methods.

  /// Return a reference to the <object_addr>.
  const ACE_INET_Addr &object_addr () const;

  /// Return a pointer to the host string.  This object maintains
  /// ownership of this string.
  const char *host () const;

  /// Copy the string @a h into <host_> and return the resulting pointer.
  /// This object maintains ownership of this string.
  const char *host (const char *h);

  /// Return the port number.
  CORBA::UShort port () const;

  /// Set the port number.
  CORBA::UShort port (CORBA::UShort p);

  /// Do we have a preferred local network for the target?
  bool is_preferred_network () const;

  /// Return the preferred network if any.
  const char *preferred_network () const;

#if defined (ACE_HAS_IPV6)
  /// Does the host string represent an IPv6 decimal address.
  bool is_ipv6_decimal () const;
#endif /* ACE_HAS_IPV6 */

  /// Need to have an assignment operator since the IIOP_Profile class may
  /// have to reorder its list of endpoints based on filtering by the EndpointPolicy.
  TAO_IIOP_Endpoint & operator= (const TAO_IIOP_Endpoint& other);

  /// Given a comma separated list of preferred interface directives, which
  /// are of the form <wild_remote>=<wild_local>, this function will retrieve
  /// the list of preferred local ip addresses by matching wild_local against
  /// the list of all local ip interfaces, for any directive where wild_remote
  /// matches the host from our endpoint.
  static void find_preferred_interfaces (const ACE_CString& host,
                                         const ACE_CString& csvPreferred,
                                         ACE_Vector<ACE_CString>& preferred);

private:
  TAO_IIOP_Endpoint *next_filtered_i (TAO_IIOP_Endpoint *root,
                                      bool ipv6_only,
                                      bool prefer_ipv6,
                                      bool want_ipv6);

  /// Helper method for setting INET_Addr.
  int set (const ACE_INET_Addr &addr,
           int use_dotted_decimal_addresses);

  /// Helper method for object_addr () call.
  void object_addr_i () const;

  /// Generate preferred interfaces from the options passed in by the
  /// user.
  CORBA::ULong preferred_interfaces (const char *csvPreferred,
                                     bool enforce,
                                     TAO_IIOP_Profile &profile);

  /// Chain a new duplicate of ourself with the specified
  /// local preferred interface.
  TAO_IIOP_Endpoint *add_local_endpoint (TAO_IIOP_Endpoint *ep,
                                         const char *local,
                                         TAO_IIOP_Profile &profile);

  /// Canonical copy constructor
  /**
   * In private section to prevent clients from invoking this
   * accidentally. Clients should only use duplicate () to make a deep
   * copy
   */
  TAO_IIOP_Endpoint (const TAO_IIOP_Endpoint &);

private:
  /// String representing the host name.
  CORBA::String_var host_;

  /// TCP port number.
  CORBA::UShort port_;

#if defined (ACE_HAS_IPV6)
  /// Does the host string represent an IPv6 decimal address.
  bool is_ipv6_decimal_;
#endif /* ACE_HAS_IPV6 */

  /// Is this endpoint created encodable as part of the IOR?
  bool is_encodable_;

  /// Flag to indicate if the address has been resolved and set.
  mutable bool object_addr_set_;

  /// Cached instance of ACE_INET_Addr for use in making
  /// invocations, etc.
  mutable ACE_INET_Addr object_addr_;

  /// Preferred path for this endpoint.
  TAO::IIOP_Endpoint_Info preferred_path_;

  /// IIOP Endpoints can be strung into a list.  Return the next
  /// endpoint in the list, if any.
  TAO_IIOP_Endpoint *next_;
};

TAO_END_VERSIONED_NAMESPACE_DECL

#if defined (__ACE_INLINE__)
# include "tao/IIOP_Endpoint.inl"
#endif /* __ACE_INLINE__ */

#endif /* TAO_HAS_IIOP && TAO_HAS_IIOP != 0 */

#include /**/ "ace/post.h"
#endif  /* TAO_IIOP_PROFILE_H */
