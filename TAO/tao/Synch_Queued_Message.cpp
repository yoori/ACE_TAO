// -*- C++ -*-
#include "tao/Synch_Queued_Message.h"
#include "tao/debug.h"
#include "tao/ORB_Core.h"

#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

TAO_Synch_Queued_Message::TAO_Synch_Queued_Message (
  const ACE_Message_Block *contents,
  TAO_ORB_Core *oc,
  ACE_Allocator *alloc,
  bool is_heap_allocated)
  : TAO_Queued_Message (oc, alloc, is_heap_allocated)
  , contents_ (const_cast<ACE_Message_Block*> (contents))
  , current_block_ (contents_)
  , own_contents_ (is_heap_allocated)
{
}

TAO_Synch_Queued_Message::~TAO_Synch_Queued_Message ()
{
  if (this->own_contents_ && this->contents_ != nullptr)
    {
      ACE_Message_Block::release (this->contents_);
    }
}

const ACE_Message_Block *
TAO_Synch_Queued_Message::current_block () const
{
  return this->current_block_;
}

size_t
TAO_Synch_Queued_Message::message_length () const
{
  if (this->current_block_ == nullptr)
    {
      return 0;
    }

  return this->current_block_->total_length ();
}

int
TAO_Synch_Queued_Message::all_data_sent () const
{
  return this->current_block_ == nullptr;
}

void
TAO_Synch_Queued_Message::fill_iov (int iovcnt_max,
                                    int &iovcnt,
                                    iovec iov[]) const
{
  ACE_ASSERT (iovcnt_max > iovcnt);

  for (const ACE_Message_Block *message_block = this->current_block_;
       message_block != nullptr && iovcnt < iovcnt_max;
       message_block = message_block->cont ())
    {
      size_t const message_block_length = message_block->length ();

      // Check if this block has any data to be sent.
      if (message_block_length > 0)
        {
              // Collect the data in the iovec.
          iov[iovcnt].iov_base = message_block->rd_ptr ();
          iov[iovcnt].iov_len  = static_cast<u_long> (message_block_length);

          // Increment iovec counter.
          ++iovcnt;
        }
    }
}

void
TAO_Synch_Queued_Message::bytes_transferred (size_t &byte_count)
{
  this->state_changed_i (TAO_LF_Event::LFS_ACTIVE);

  while (this->current_block_ != nullptr && byte_count > 0)
    {
      size_t const l = this->current_block_->length ();

      if (byte_count < l)
        {
          this->current_block_->rd_ptr (byte_count);
          byte_count = 0;
          return;
        }

      byte_count -= l;
      this->current_block_->rd_ptr (l);
      this->current_block_ = this->current_block_->cont ();

      while (this->current_block_ != nullptr
             && this->current_block_->length () == 0)
        {
          this->current_block_ = this->current_block_->cont ();
        }
    }

  if (this->current_block_ == nullptr)
    this->state_changed (TAO_LF_Event::LFS_SUCCESS,
                         this->orb_core_->leader_follower ());
}

TAO_Queued_Message *
TAO_Synch_Queued_Message::clone (ACE_Allocator *alloc)
{
  TAO_Synch_Queued_Message *qm = nullptr;

  // Clone the message block.
  // NOTE: We wantedly do the cloning from <current_block_> instead of
  // starting from <contents_> since we dont want to clone blocks that
  // have already been sent on the wire. Waste of memory and
  // associated copying.
  ACE_Message_Block *mb = 0;
  ACE_NEW_NORETURN (mb,
                    ACE_Message_Block (0, // size
                                       ACE_Message_Block::ACE_Message_Type (0), // type
                                       0, // cont
                                       0, // data
                                       0, // allocator
                                       0, // locking strategy
                                       this->current_block_->msg_priority(), // priority
                                       this->current_block_->msg_execution_time(), // execution time
                                       this->current_block_->msg_deadline_time(), // absolute time to deadline
                                       this->current_block_->data_block()->data_block_allocator (),
                                       0));
  if (!mb)
    {
      return 0;
    }
 
  int result = ACE_CDR::consolidate (mb, this->current_block_);
  if (result == -1)
    {
      mb->release ();
      return 0;
    }


  if (alloc)
    {
      ACE_NEW_MALLOC_NORETURN (qm,
                               static_cast<TAO_Synch_Queued_Message *> (
                                 alloc->malloc (sizeof (TAO_Synch_Queued_Message))),
                               TAO_Synch_Queued_Message (mb,
                                                         this->orb_core_,
                                                         alloc,
                                                         true));
    }
  else
    {
      ACE_NEW_NORETURN (qm,
                        TAO_Synch_Queued_Message (mb, this->orb_core_, 0, true));
    }

  if (!qm)
    {
      mb->release ();
    }

  return qm;
}

void
TAO_Synch_Queued_Message::destroy ()
{
  if (this->own_contents_)
    {
      ACE_Message_Block::release (this->contents_);
      this->current_block_ = nullptr;
      this->contents_ = nullptr;
    }

  if (this->is_heap_created_)
    {
      // If we have an allocator release the memory to the allocator
      // pool.
      if (this->allocator_)
        {
          ACE_DES_FREE_THIS (this->allocator_->free,
                             TAO_Synch_Queued_Message);
        }
      else // global release..
        {
          delete this;
        }
    }
}

void
TAO_Synch_Queued_Message::copy_if_necessary (const ACE_Message_Block* chain)
{
  if (!this->own_contents_)
    {
      // Go through the message block chain looking for the message block
      // that matches our "current" message block.
      for (const ACE_Message_Block* mb = chain; mb != nullptr; mb = mb->cont ())
        {
          if (mb == this->current_block_)
            {
              // Once we have found the message block, we need to
              // clone the current block so that if another thread comes
              // in and calls reset() on the output stream (via another
              // invocation on the transport), it doesn't cause the rest
              // of our message to be released.
              this->own_contents_ = true;
              this->contents_ = this->current_block_->clone ();
              this->current_block_ = this->contents_;
              break;
            }
        }
    }
}

TAO_END_VERSIONED_NAMESPACE_DECL
