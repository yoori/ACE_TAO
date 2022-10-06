// -*- C++ -*-
TAO_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE TAO_Profile*
TAO_Profile::_duplicate (TAO_Profile* obj)
{
  if (obj)
  {
    obj->_incr_refcount ();
  }

  return obj;
}

ACE_INLINE CORBA::ULong
TAO_Profile::tag () const
{
  return this->tag_;
}

ACE_INLINE const TAO_GIOP_Message_Version &
TAO_Profile::version () const
{
  return this->version_;
}

ACE_INLINE TAO_ORB_Core *
TAO_Profile::orb_core () const
{
  return this->orb_core_;
}

ACE_INLINE void
TAO_Profile::forward_to (TAO_MProfile *mprofiles)
{
  this->forward_to_ = mprofiles;
}

ACE_INLINE TAO_MProfile *
TAO_Profile::forward_to ()
{
  return this->forward_to_;
}

ACE_INLINE TAO_MProfile *
TAO_Profile::forward_to_i ()
{
  return this->forward_to_;
}

ACE_INLINE const TAO_Tagged_Components&
TAO_Profile::tagged_components () const
{
  return this->tagged_components_;
}

ACE_INLINE TAO_Tagged_Components&
TAO_Profile::tagged_components ()
{
  return this->tagged_components_;
}

ACE_INLINE CORBA::Short
TAO_Profile::addressing_mode () const
{
  return this->addressing_mode_;
}

ACE_INLINE const TAO::ObjectKey &
TAO_Profile::object_key () const
{
  return this->ref_object_key_->object_key ();
}

ACE_INLINE unsigned long
TAO_Profile::_incr_refcount (void)
{
  return ++this->refcount_;
}

ACE_INLINE unsigned long
TAO_Profile::_incr_refcnt ()
{
  return this->_incr_refcount ();
}

ACE_INLINE unsigned long
TAO_Profile::_decr_refcount ()
{
  unsigned long count = --this->refcount_;
  if (count == 0)
    {
      // refcount is 0, so delete us!
      // delete will call our ~ destructor which in turn deletes stuff.
      delete this;
    }
  return count;
}

ACE_INLINE unsigned long
TAO_Profile::_decr_refcnt (void)
{
  return this->_decr_refcount ();
}

TAO_END_VERSIONED_NAMESPACE_DECL
