#include "server_i.h"

server_i::server_i (int quiet,
                    CORBA::ORB_ptr orb)
  : quiet_ (quiet),
    orb_ (CORBA::ORB::_duplicate (orb))
{
}

void
server_i::start (client_ptr c,
                 CORBA::UShort time_to_live)
{
  this->client_ = client::_duplicate (c);
  this->ping (time_to_live);

  return;
}

void
server_i::ping (CORBA::UShort time_to_live)
{
  if (!this->quiet_)
    ACE_DEBUG ((LM_DEBUG,
                "(%t) server_i::ping -> time to live = %d\n",
                time_to_live));

  --time_to_live;

  if (time_to_live > 0)
    {
      this->client_->ping (time_to_live);
    }
}

void
server_i::shutdown ()
{
  this->orb_->shutdown (false);
}
