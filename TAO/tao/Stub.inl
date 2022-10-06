// -*- C++ -*-
#include "tao/ORB_Core.h"
#include "ace/Reverse_Lock_T.h"

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE void
TAO_Stub::reset_base_i ()
{
  this->base_profiles_.rewind ();
  this->profile_success_ = false;

  this->set_profile_in_use_i (base_profiles_.get_next ());
}

ACE_INLINE const TAO_SYNCH_MUTEX&
TAO_Stub::profile_lock () const
{
  return this->profile_lock_;
}

ACE_INLINE void
TAO_Stub::reset_forward_i ()
{
  while (this->forward_profiles_ != 0
         && this->forward_profiles_ != this->forward_profiles_perm_) // Disturbingly the permanent
                                                                     // forwarded profile lives at the  bottom
                                                                     // of this stack if it exists. Avoid deleting it.
    this->forward_back_one_i ();
}

ACE_INLINE void
TAO_Stub::reset_profiles_i ()
{
  this->reset_forward_i ();
  this->reset_base_i ();

  if (this->forward_profiles_perm_)
    {
      // The *permanent* forward is being kept in the transient
      // forward queue (??!). We have just nuked it. Put it back the way it was.
      // reset_base should have reset the profile success.
      // @todo We have knives in the spoon draw - TAO_Stub needs total rewrite.
      this->forward_profiles_ = this->forward_profiles_perm_;
      this->forward_profiles_->rewind ();
      this->set_profile_in_use_i (this->forward_profiles_->get_next ());
    }
}

ACE_INLINE void
TAO_Stub::reset_profiles ()
{
  ACE_MT (ACE_GUARD (TAO_SYNCH_MUTEX,
                     guard,
                     this->profile_lock_));

  if (TAO_debug_level > 5)
    {
      TAOLIB_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("TAO (%P|%t) - Stub::reset_profiles, acquired ")
                  ACE_TEXT ("profile lock this = 0x%x\n"),
                  this));
    }

  this->reset_profiles_i ();
}

ACE_INLINE TAO_Profile *
TAO_Stub::profile_in_use_ptr_i ()
{
  return this->profile_in_use_;
}

ACE_INLINE TAO_Profile *
TAO_Stub::profile_in_use_ptr ()
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_in_use_lock_,
                            0));

  return this->profile_in_use_ptr_i();
}

ACE_INLINE TAO_MProfile *
TAO_Stub::make_profiles ()
{
  TAO_MProfile *mp = 0;

  ACE_NEW_RETURN (mp,
                  TAO_MProfile (base_profiles_),
                  0);

  return mp;
}

ACE_INLINE TAO_MProfile *
TAO_Stub::make_forward_profiles (void)
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_lock_,
                            0));

  TAO_MProfile *mp = 0;

  if(forward_profiles_)
  {
    ACE_NEW_RETURN (mp,
                    TAO_MProfile (*forward_profiles_),
                    0);
  }

  return mp;
}

ACE_INLINE TAO_Profile *
TAO_Stub::next_forward_profile_i ()
{
  TAO_Profile *pfile_next = 0;

  while (this->forward_profiles_
         && (pfile_next = this->forward_profiles_->get_next ()) == 0
         && this->forward_profiles_ != this->forward_profiles_perm_)  // do not remove permanent forward from bottom of stack
    // that was the last profile.  Now we clean up our forward profiles.
    // since we own the forward MProfiles, we must delete them when done.
    this->forward_back_one_i ();

  return pfile_next;
}

ACE_INLINE bool
TAO_Stub::next_profile ()
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_lock_,
                            0));
  if (TAO_debug_level > 5)
    {
      TAOLIB_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("TAO (%P|%t) - Stub::next_profile, acquired profile lock this = 0x%x\n"), this));
    }

  return this->next_profile_i ();
}

ACE_INLINE CORBA::Boolean
TAO_Stub::valid_forward_profile ()
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_lock_,
                            false));
  return (this->profile_success_ && this->forward_profiles_);
}

ACE_INLINE void
TAO_Stub::set_valid_profile ()
{
  this->profile_success_ = true;
}

ACE_INLINE CORBA::Boolean
TAO_Stub::valid_profile () const
{
  return this->profile_success_;
}

ACE_INLINE void
TAO_Stub::base_profiles (const TAO_MProfile &mprofiles)
{
  ACE_MT (ACE_GUARD (TAO_SYNCH_MUTEX,
                     guard,
                     this->profile_lock_));
  if (TAO_debug_level > 5)
    {
      TAOLIB_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("TAO (%P|%t) - Stub::base_profiles, acquired profile lock this = 0x%x\n"), this));
    }


  // first reset things so we start from scratch!

  // @note This reset forward could effect the collocation status
  // but as this method is only used from the Stub ctr, when the status
  // is already correctly set, we don't reinitialise here. sm.
  this->reset_forward_i ();
  this->base_profiles_.set (mprofiles);
  this->reset_base_i ();
}

ACE_INLINE CORBA::Boolean
TAO_Stub::next_profile_retry_i ()
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_lock_,
                            0));
  if (TAO_debug_level > 5)
    {
      TAOLIB_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("TAO (%P|%t) - Stub::next_profile_retry, acquired profile lock this = 0x%x\n"), this));
    }

  if (this->profile_success_ && this->forward_profiles_)
    {
      // We have a forwarded reference that we have managed to *send* a message to
      // previously in the remote path only (but not counting object proxy broker ops).
      // @todo I can see little sense to this. It is at best highly inconsistent. sm.

      // In this case we are falling back from the forwarded IOR stright to the base IOR
      this->reset_profiles_i ();

      // We used to return unconditional true at this point but this results in
      // infinite retries of any permanent location forward. This is undesirable.
      return !this->forward_profiles_perm_;
    }
  else if (this->next_profile_i ())
    {
      return true;
    }

  return false;
}

ACE_INLINE CORBA::Boolean
TAO_Stub::next_profile_retry (void)
{
  ACE_MT (ACE_GUARD_RETURN (TAO_SYNCH_MUTEX,
                            guard,
                            this->profile_lock_,
                            0));

  return this->next_profile_retry_i();
}

ACE_INLINE const TAO_MProfile&
TAO_Stub::base_profiles () const
{
  return this->base_profiles_;
}

ACE_INLINE TAO_MProfile&
TAO_Stub::base_profiles ()
{
  return this->base_profiles_;
}

ACE_INLINE const TAO_MProfile *
TAO_Stub::forward_profiles_i () const
{
  return this->forward_profiles_;
}

/*
ACE_INLINE TAO_MProfile *
TAO_Stub::forward_profiles ()
{
  return this->forward_profiles_;
}
*/

ACE_INLINE CORBA::Boolean
TAO_Stub::is_collocated () const
{
  return this->is_collocated_;
}

ACE_INLINE TAO_ORB_Core*
TAO_Stub::orb_core () const
{
  return this->orb_core_.get ();
}

ACE_INLINE CORBA::ORB_var &
TAO_Stub::servant_orb_var ()
{
  // Simply pass back the ORB pointer for temporary use.
  return this->servant_orb_;
}

ACE_INLINE CORBA::ORB_ptr
TAO_Stub::servant_orb_ptr ()
{
  // Simply pass back the ORB pointer for temporary use.
  return CORBA::ORB::_duplicate (this->servant_orb_.in ());
}

ACE_INLINE void
TAO_Stub::servant_orb (CORBA::ORB_ptr orb)
{
  this->servant_orb_ = CORBA::ORB::_duplicate (orb);
}

ACE_INLINE TAO_Abstract_ServantBase *
TAO_Stub::collocated_servant () const
{
  return collocated_servant_;
}

ACE_INLINE void
TAO_Stub::collocated_servant (TAO_Abstract_ServantBase * servant)
{
  this->collocated_servant_ = servant;
}

ACE_INLINE TAO::Object_Proxy_Broker *
TAO_Stub::object_proxy_broker () const
{
  return this->object_proxy_broker_;
}

ACE_INLINE void
TAO_Stub::object_proxy_broker (TAO::Object_Proxy_Broker * object_proxy_broker)
{
  this->object_proxy_broker_ = object_proxy_broker;
}

ACE_INLINE void
TAO_Stub::destroy ()
{
  // The reference count better be zero at this point!
  delete this;
}

ACE_INLINE CORBA::Boolean
TAO_Stub::optimize_collocation_objects () const
{
  return this->collocation_opt_;
}

ACE_INLINE TAO::Transport_Queueing_Strategy *
TAO_Stub::transport_queueing_strategy ()
{
#if (TAO_HAS_BUFFERING_CONSTRAINT_POLICY == 1)

  bool has_synchronization;
  Messaging::SyncScope scope;

  this->orb_core_->call_sync_scope_hook (this, has_synchronization, scope);

  if (has_synchronization == true)
    return this->orb_core_->get_transport_queueing_strategy  (this, scope);
#endif /* TAO_HAS_BUFFERING_CONSTRAINT_POLICY == 1 */

  // No queueing strategy, let the transport use its default
  return 0;
}

ACE_INLINE
void TAO_Stub::forwarded_on_exception (bool forwarded)
{
  forwarded_on_exception_ = forwarded;
}

ACE_INLINE
bool TAO_Stub::forwarded_on_exception () const
{
  return forwarded_on_exception_;
}

ACE_INLINE
void
TAO_Stub::_incr_refcnt ()
{
  ++this->refcount_;
}

ACE_INLINE
void
TAO_Stub::_decr_refcnt ()
{
  if (--this->refcount_ == 0)
    delete this;
}

ACE_INLINE
CORBA::Boolean
TAO_Stub::at_starting_profile () const
{
  return profile_in_use_ == base_profiles_.get_profile(0);
}

// ---------------------------------------------------------------

// Creator methods for TAO_Stub_Auto_Ptr (TAO_Stub Auto Pointer)
ACE_INLINE
TAO_Stub_Auto_Ptr::TAO_Stub_Auto_Ptr (TAO_Stub *p)
  : p_ (p)
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::TAO_Stub_Auto_Ptr");
}

ACE_INLINE TAO_Stub *
TAO_Stub_Auto_Ptr::get () const
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::get");
  return this->p_;
}

ACE_INLINE TAO_Stub *
TAO_Stub_Auto_Ptr::release ()
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::release");
  TAO_Stub *old = this->p_;
  this->p_ = nullptr;
  return old;
}

ACE_INLINE void
TAO_Stub_Auto_Ptr::reset (TAO_Stub *p)
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::reset");
  if (this->get () != p && this->get () != nullptr)
    this->get ()->_decr_refcnt ();
  this->p_ = p;
}

ACE_INLINE TAO_Stub *
TAO_Stub_Auto_Ptr::operator-> () const
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::operator->");
  return this->get ();
}

ACE_INLINE
TAO_Stub_Auto_Ptr::TAO_Stub_Auto_Ptr (TAO_Stub_Auto_Ptr &rhs)
  : p_ (rhs.release ())
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::TAO_Stub_Auto_Ptr");
}

ACE_INLINE TAO_Stub_Auto_Ptr &
TAO_Stub_Auto_Ptr::operator= (TAO_Stub_Auto_Ptr &rhs)
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::operator=");
  if (this != &rhs)
    {
      this->reset (rhs.release ());
    }
  return *this;
}

ACE_INLINE
TAO_Stub_Auto_Ptr::~TAO_Stub_Auto_Ptr ()
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::~TAO_Stub_Auto_Ptr");
  if (this->get() != nullptr)
    this->get ()->_decr_refcnt ();
}

// Accessor methods to the underlying Stub Object
ACE_INLINE TAO_Stub &
TAO_Stub_Auto_Ptr::operator *() const
{
  ACE_TRACE ("TAO_Stub_Auto_Ptr::operator *()");
  // @@ Potential problem if this->p_ is zero!
  return *this->get ();
}

TAO_END_VERSIONED_NAMESPACE_DECL
