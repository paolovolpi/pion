// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2012 Cloudmeter, Inc.  (http://www.cloudmeter.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include <pion/tcp/timer.hpp>
#include <boost/bind.hpp>


namespace pion {    // begin namespace pion
namespace tcp {     // begin namespace tcp


// timer member functions

timer::timer(tcp::connection_ptr& conn_ptr)
    : m_conn_ptr(conn_ptr), m_timer(conn_ptr->get_io_service()),
    m_timer_active(false), m_was_cancelled(false)
{
}

void timer::start(const boost::uint32_t seconds)
{
    boost::mutex::scoped_lock timer_lock(m_mutex);
    m_timer_active = true;
    m_timer.expires_from_now(boost::posix_time::seconds(seconds));
    m_timer.async_wait(boost::bind(&timer::timer_callback,
        shared_from_this(), _1));
}

void timer::cancel(void)
{
    boost::mutex::scoped_lock timer_lock(m_mutex);
    m_was_cancelled = true;
    if (m_timer_active)
        m_timer.cancel();
}

void timer::timer_callback(const boost::system::error_code& ec)
{
    boost::mutex::scoped_lock timer_lock(m_mutex);
    m_timer_active = false;
    if (! m_was_cancelled)
        m_conn_ptr->cancel();
}


}   // end namespace tcp
}   // end namespace pion
