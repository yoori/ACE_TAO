// -*- C++ -*-

//==========================================================================
/**
 *  @file    Guard_T.h
 *
 *  @author Douglas C. Schmidt <d.schmidt@vanderbilt.edu>
 */
//==========================================================================

#ifndef ACE_GUARD_T_H
#define ACE_GUARD_T_H
#include /**/ "ace/pre.h"

#include "ace/Lock.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/Global_Macros.h"
#include "ace/OS_NS_Thread.h"

// FUZZ: disable check_for_ACE_Guard

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

/**
 * @class ACE_Guard
 *
 * @brief This data structure is meant to be used within a method or
 * function...  It performs automatic acquisition and release of
 * a parameterized synchronization object ACE_LOCK.
 *
 * The ACE_LOCK class given as an actual parameter must provide, at
 * the very least the acquire(), tryacquire(), release(), and
 * remove() methods.
 *
 * @warning A successfully constructed ACE_Guard does NOT mean that the
 * lock was acquired!  It is the caller's responsibility, after
 * constructing an ACE_Guard, to check whether the lock was successfully
 * acquired.  Code like this is dangerous:
 *   {
 *     ACE_Guard<ACE_Lock> g(lock);
 *     ... perform critical operation requiring lock to be held ...
 *   }
 * Instead, one must do something like this:
 *   {
 *     ACE_Guard<ACE_Lock> g(lock);
 *     if (! g.locked())
 *       {
 *         ... handle error ...
 *       }
 *     else
 *       {
 *         ... perform critical operation requiring lock to be held ...
 *       }
 *   }
 * The ACE_GUARD_RETURN() and ACE_GUARD_REACTION() macros are designed to
 * to help with this.
 */
template <class ACE_LOCK>
class ACE_Guard
{
public:
  ACE_Guard (ACE_LOCK &l);

  /// Implicitly and automatically acquire (or try to acquire) the
  /// lock.  If @a block is non-0 then acquire() the ACE_LOCK, else
  /// tryacquire() it.
  ACE_Guard (ACE_LOCK &l, bool block);

  /// Initialize the guard without implicitly acquiring the lock. The
  /// @a become_owner parameter indicates whether the guard should release
  /// the lock implicitly on destruction. The @a block parameter is
  /// ignored and is used here to disambiguate with the preceding
  /// constructor.
  ACE_Guard (ACE_LOCK &l, bool block, int become_owner);

  /// Implicitly release the lock.
  ~ACE_Guard ();

  // = Lock accessors.

  /// Explicitly acquire the lock.
  int acquire ();

  /// Conditionally acquire the lock (i.e., won't block).
  int tryacquire ();

  /// Explicitly release the lock, but only if it is held!
  int release ();

  /// Relinquish ownership of the lock so that it is not released
  /// implicitly in the destructor.
  void disown ();

  // = Utility methods.
  /// true if locked, false if couldn't acquire the lock
  /// (errno will contain the reason for this).
  bool locked () const;

  /// Explicitly remove the lock.
  int remove ();

  /// Dump the state of an object.
  void dump () const;

  // ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.

protected:
  /// Helper, meant for subclass only.
  ACE_Guard (ACE_LOCK *lock): lock_ (lock), owner_ (0) {}

  /// Pointer to the ACE_LOCK we're guarding.
  ACE_LOCK *lock_;

  /// Keeps track of whether we acquired the lock or failed.
  int owner_;

private:
  void operator= (const ACE_Guard<ACE_LOCK> &) = delete;
  ACE_Guard (const ACE_Guard<ACE_LOCK> &) = delete;
};

/**
 * @class ACE_Write_Guard
 *
 * @brief This class is similar to class ACE_Guard, though it
 * acquires/releases a write lock automatically (naturally, the
 * <ACE_LOCK> it is instantiated with must support the appropriate
 * API).
 *
 * @warning See important "WARNING" in comments at top of ACE_Guard.
 */
template <class ACE_LOCK>
class ACE_Write_Guard : public ACE_Guard<ACE_LOCK>
{
public:
  /// Implicitly and automatically acquire a write lock.
  ACE_Write_Guard (ACE_LOCK &m);

  /// Implicitly and automatically acquire (or try to acquire) a write
  /// lock.
  ACE_Write_Guard (ACE_LOCK &m, bool block);

  // = Lock accessors.

  /// Explicitly acquire the write lock.
  int acquire_write ();

  /// Explicitly acquire the write lock.
  int acquire ();

  /// Conditionally acquire the write lock (i.e., won't block).
  int tryacquire_write ();

  /// Conditionally acquire the write lock (i.e., won't block).
  int tryacquire ();

  // = Utility methods.

  /// Dump the state of an object.
  void dump () const;

  // ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.
};

/**
 * @class ACE_Read_Guard
 *
 * @brief This class is similar to class ACE_Guard, though it
 * acquires/releases a read lock automatically (naturally, the
 * <ACE_LOCK> it is instantiated with must support the appropriate
 * API).
 *
 * @warning See important "WARNING" in comments at top of ACE_Guard.
 */
template <class ACE_LOCK>
class ACE_Read_Guard : public ACE_Guard<ACE_LOCK>
{
public:
  /// Implicitly and automatically acquire a read lock.
  ACE_Read_Guard (ACE_LOCK& m);

  /// Implicitly and automatically acquire (or try to acquire) a read
  /// lock.
  ACE_Read_Guard (ACE_LOCK &m, bool block);

  // = Lock accessors.

  /// Explicitly acquire the read lock.
  int acquire_read ();

  /// Explicitly acquire the read lock.
  int acquire ();

  /// Conditionally acquire the read lock (i.e., won't block).
  int tryacquire_read ();

  /// Conditionally acquire the read lock (i.e., won't block).
  int tryacquire ();

  // = Utility methods.

  /// Dump the state of an object.
  void dump () const;

  // ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.
};

#if !(defined (ACE_HAS_THREADS) && (defined (ACE_HAS_THREAD_SPECIFIC_STORAGE) || defined (ACE_HAS_TSS_EMULATION)))

#define ACE_TSS_Guard ACE_Guard
#define ACE_TSS_Write_GUARD ACE_Write_Guard
#define ACE_TSS_Read_GUARD ACE_Read_Guard

#else
 /* ACE platform supports some form of threading and
  thread-specific storage. */

/**
 * @class ACE_TSS_Guard
 *
 * @brief This data structure is meant to be used within a method or
 * function...  It performs automatic aquisition and release of
 * a synchronization object.  Moreover, it ensures that the lock
 * is released even if a thread exits via <thr_exit>!
 */
template <class ACE_LOCK>
class ACE_TSS_Guard
{
public:
  /// Implicitly and automatically acquire the thread-specific lock.
  ACE_TSS_Guard (ACE_LOCK &lock, bool block = true);

  /// Implicitly release the thread-specific lock.
  ~ACE_TSS_Guard ();

  // = Lock accessors.

  /// Explicitly acquire the thread-specific lock.
  int acquire ();

  /// Conditionally acquire the thread-specific lock (i.e., won't
  /// block).
  int tryacquire ();

  /// Explicitly release the thread-specific lock.
  int release ();

  // = Utility methods.
  /// Explicitly release the thread-specific lock.
  int remove ();

  /// Dump the state of an object.
  void dump () const;

  /// Declare the dynamic allocation hooks.
  ACE_ALLOC_HOOK_DECLARE;

protected:
  /// Helper, meant for subclass only.
  ACE_TSS_Guard ();

  /// Initialize the key.
  void init_key ();

  /// Called when thread exits to clean up the lock.
  static void cleanup (void *ptr);

  /// Thread-specific key...
  ACE_thread_key_t key_;

private:
  // FUZZ: disable check_for_ACE_Guard
  typedef ACE_Guard<ACE_LOCK> Guard_Type;
  // FUZZ: enable check_for_ACE_Guard

  void operator= (const ACE_TSS_Guard<ACE_LOCK> &) = delete;
  ACE_TSS_Guard (const ACE_TSS_Guard<ACE_LOCK> &) = delete;
};

/**
 * @class ACE_TSS_Write_Guard
 *
 * @brief This class is similar to class ACE_TSS_Guard, though it
 * acquires/releases a write-lock automatically (naturally, the
 * ACE_LOCK it is instantiated with must support the appropriate
 * API).
 */
template <class ACE_LOCK>
class ACE_TSS_Write_Guard : public ACE_TSS_Guard<ACE_LOCK>
{
public:
  /// Implicitly and automatically acquire the thread-specific write lock.
  ACE_TSS_Write_Guard (ACE_LOCK &lock, bool block = true);

  // = Lock accessors.

  /// Explicitly acquire the thread-specific write lock.
  int acquire_write ();

  /// Explicitly acquire the thread-specific write lock.
  int acquire ();

  /// Conditionally acquire the thread-specific write lock (i.e., won't block).
  int tryacquire_write ();

  /// Conditionally acquire the thread-specific write lock (i.e., won't block).
  int tryacquire ();

  // = Utility methods.

  /// Dump the state of an object.
  void dump () const;

  // ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.
private:
  // FUZZ: disable check_for_ACE_Guard
  typedef ACE_Guard<ACE_LOCK> Guard_Type;
  typedef ACE_Write_Guard<ACE_LOCK> Write_Guard_Type;
  // FUZZ: enable check_for_ACE_Guard
};

/**
 * @class ACE_TSS_Read_Guard
 *
 * @brief This class is similar to class <ACE_TSS_Guard>, though it
 * acquires/releases a read lock automatically (naturally, the
 * <ACE_LOCK> it is instantiated with must support the
 * appropriate API).
 */
template <class ACE_LOCK>
class ACE_TSS_Read_Guard : public ACE_TSS_Guard<ACE_LOCK>
{
public:
  /// Implicitly and automatically acquire the thread-specific read lock.
  ACE_TSS_Read_Guard (ACE_LOCK &lock, bool block = true);

  // = Lock accessors.
  /// Explicitly acquire the thread-specific read lock.
  int acquire_read ();

  /// Explicitly acquire the thread-specific read lock.
  int acquire ();

  /// Conditionally acquire the thread-specific read lock (i.e., won't
  /// block).
  int tryacquire_read ();

  /// Conditionally acquire the thread-specific read lock (i.e., won't
  /// block).
  int tryacquire ();

  // = Utility methods.
  /// Dump the state of an object.
  void dump () const;

  // ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.
private:
  // FUZZ: disable check_for_ACE_Guard
  typedef ACE_Guard<ACE_LOCK> Guard_Type;
  typedef ACE_Read_Guard<ACE_LOCK> Read_Guard_Type;
  // FUZZ: enable check_for_ACE_Guard
};

#endif /* !(defined (ACE_HAS_THREADS) && (defined (ACE_HAS_THREAD_SPECIFIC_STORAGE) || defined (ACE_HAS_TSS_EMULATION))) */

ACE_END_VERSIONED_NAMESPACE_DECL

#if defined (__ACE_INLINE__)
#include "ace/Guard_T.inl"
#endif /* __ACE_INLINE__ */

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
#include "ace/Guard_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
#pragma implementation ("Guard_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#include /**/ "ace/post.h"
#endif /* ACE_GUARD_T_H */
