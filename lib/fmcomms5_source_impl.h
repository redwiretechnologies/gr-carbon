/* -*- c++ -*- */
/*
 * Copyright 2019 Red Wire Technologies.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_CARBON_FMCOMMS_SOURCE_IMPL_H
#define INCLUDED_CARBON_FMCOMMS_SOURCE_IMPL_H

#include <list>

#include <gnuradio/carbon/fmcomms5_source.h>
#include "rwt_base_block_dual_impl.h"

namespace gr {
namespace rwt {

class fmcomms5_source_impl : public fmcomms5_source, public rwt_base_block_dual_impl
{
public:
    fmcomms5_source_impl(
        const pmt::pmt_t config,
        unsigned int reg_base_addr,
        const char *filter,
        bool auto_filter,
        bool force_reload,
        unsigned int buffer_size);


    ~fmcomms5_source_impl();

    int work(
        int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items);

    void ctrl_reg_handler(
        const std::string key,
        const std::string value);

    bool start();
    bool stop();

private:
    void alloc_volk_buffers(int nitems);

    void free_volk_buffers() {
        if (m_pkt_data) volk_free(m_pkt_data);
        if (m_data_interleaved) volk_free(m_data_interleaved);
        m_pkt_data = NULL;
        m_data_interleaved = NULL;
        m_alloc_len = 0;
    }

    bool m_in_constructor;
    bool m_autofilter;
    bool running;

    unsigned long m_samplerate;

    size_t m_alloc_len;
    short *m_pkt_data;
    float *m_data_interleaved;

};

} // namespace rwt
} // namespace gr

#endif /* INCLUDED_CARBON_FMCOMMS_SOURCE_IMPL_H */
