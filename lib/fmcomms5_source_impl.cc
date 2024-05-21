/* -*- c++ -*- */
/*
 * Copyright 2019 $Red Wire Technologies.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ad9361.h>
#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include <unistd.h>
#include "fmcomms5_source_impl.h"

namespace gr {
namespace rwt {

#define DEBUG 0

fmcomms5_source::sptr
fmcomms5_source::make(
    const pmt::pmt_t config,
    unsigned int reg_base_addr,
    const char *filter,
    bool auto_filter,
    bool force_reload,
    unsigned int buffer_size)
{
  return gnuradio::get_initial_sptr
    (new fmcomms5_source_impl(
        config,
        reg_base_addr,
        filter,
        auto_filter,
        force_reload,
        buffer_size));
}

/*
 * The private constructor
 */

fmcomms5_source_impl::fmcomms5_source_impl(
    const pmt::pmt_t config,
    unsigned int reg_base_addr,
    const char *filter,
    bool auto_filter,
    bool force_reload,
    unsigned int buffer_size) :
    rwt_base_block(
        "fmcomms5_source",
        gr::io_signature::make(0, 0, 0),
        gr::io_signature::make(4, 4,
            sizeof(gr_complex))),
    rwt_base_block_dual_impl(
        "fmcomms5_source",
        gr::io_signature::make(0, 0, 0),
        gr::io_signature::make(4, 4,
            sizeof(gr_complex)),
        buffer_size,
        0,
        reg_base_addr,
        true,
        false,
        "fmcomms5",
        force_reload,
        "ad9361-phy",
        "cf-ad9361-A",
        "cf-ad9361-dds-core-A",
        "ad9361-phy-B",
        "cf-ad9361-B",
        "cf-ad9361-dds-core-B"
        ),
    m_in_constructor(true),
    m_autofilter(auto_filter),
    m_alloc_len(0),
    m_pkt_data(NULL),
    m_data_interleaved(NULL)
{

    const char *handlers[] = {
        "rx_freq"
    };

    const int alignment_multiple =
        volk_get_alignment() / sizeof(short);
    set_alignment(std::max(1, alignment_multiple));
    set_output_multiple(8);

    for (long unsigned int i = 0; i < (sizeof(handlers)/sizeof(char *)); i++) {
        set_config_handler(
            handlers[i],
            std::bind(&fmcomms5_source_impl::ctrl_reg_handler, this, std::placeholders::_1, std::placeholders::_2));
    }

    config_msg_handler(config);

    //m_source->setup_tags(false, 0);

    m_in_constructor = false;
}

/*
 * Our virtual destructor.
 */

fmcomms5_source_impl::~fmcomms5_source_impl()
{
    free_volk_buffers();
}

void
fmcomms5_source_impl::ctrl_reg_handler(
    const std::string key,
    const std::string value)
{

    int ret;
    uint64_t value_u64;

    if (key == "rx_freq") {
        value_u64 = (uint64_t)strtoull(value.c_str(), NULL, 0);
        //Set the frequency
        m_common->set_attr(key, value);
        //Stop the block
        if(!m_in_constructor)
            stop();
        //Sync
        ret = ad9361_fmcomms5_phase_sync(iio_create_local_context(), value_u64);
        printf("Return value of sync - %d\n", ret);
        //Start the block back
        if(!m_in_constructor)
            start();
    }
}

void
fmcomms5_source_impl::alloc_volk_buffers(int nitems)
{
    const unsigned int alignment = volk_get_alignment();

    free_volk_buffers();

    m_pkt_data = (short *)volk_malloc(sizeof(short) * nitems, alignment);
    if (!m_pkt_data) {
        throw std::runtime_error(
            "rwt_source: Failed to allocate volk buffers");
    }

    m_data_interleaved = (float *)volk_malloc(
        sizeof(float) * nitems, alignment);
    if (!m_data_interleaved) {
        throw std::runtime_error(
            "rwt_source: Failed to allocate volk buffers");
    }

    m_alloc_len = nitems;
}


int
fmcomms5_source_impl::work(
    int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
    gr_complex *out0 = (gr_complex *) output_items[0];
    gr_complex *out1 = (gr_complex *) output_items[1];
    gr_complex *out2 = (gr_complex *) output_items[2];
    gr_complex *out3 = (gr_complex *) output_items[3];
    bool next_tag_valid;
    uint8_t next_tag_type;
    uint64_t next_tag_value;
    size_t max_pkt_size;
    size_t pkt_idx = 0;
    size_t pkt_read_count;
    int ret;
    gr::tag_t tag;

    (void)input_items;

    max_pkt_size = noutput_items * 8;

    if (m_alloc_len < max_pkt_size) {
        alloc_volk_buffers(max_pkt_size);
    }

    while (pkt_idx < max_pkt_size) {
        ret = m_source->read_until_next_tag_or_end(
            false,
            &m_pkt_data[pkt_idx],
            2,
            max_pkt_size - pkt_idx,
            &next_tag_valid,
            &next_tag_type,
            &next_tag_value,
            &pkt_read_count);

        if (ret == -ETIMEDOUT) {
            std::cerr << "Warning: rwt_source: timed out.\n";
            break;
        } else if (ret < 0) {
            std::cerr << "Warning: iio:source Buffer refill failed.\n";
            return -1;
        }

        if (!pkt_read_count && pkt_idx) {
            /* Didn't read anything last try, but data is already available.
               So go ahead and send it. */
            break;
        }

        pkt_idx += pkt_read_count;

    }

    if (!pkt_idx)
        return 0;

    /* Convert the types from short to complex numbers. */

    /* pkt_idx is total # float samples, whereas noutput_items # complex
       samples per channel. */
    noutput_items = pkt_idx / 8;

    /* Short to float */
    volk_16i_s32f_convert_32f(
        m_data_interleaved, m_pkt_data, 2048.0, pkt_idx);

    /* The data is currently i0,q0,i1,q1, so we have to deinterleave it.
       There are no volk intrinsics to handle this (or atleast I couldn't
       find one). */
    float *data_ptr = m_data_interleaved;
    float *out0_ptr = (float *)out0;
    float *out1_ptr = (float *)out1;
    float *out2_ptr = (float *)out2;
    float *out3_ptr = (float *)out3;
    for (int i = 0; i < noutput_items; i++) {
        *out0_ptr++ = *data_ptr++;
        *out0_ptr++ = *data_ptr--;
        data_ptr+=2;
        *out1_ptr++ = *data_ptr++;
        *out1_ptr++ = *data_ptr--;
        data_ptr+=2;
        *out2_ptr++ = *data_ptr++;
        *out2_ptr++ = *data_ptr--;
        data_ptr+=2;
        *out3_ptr++ = *data_ptr++;
        *out3_ptr++ = *data_ptr--;
        data_ptr+=2;

    }

    // Tell runtime system how many output items we produced.
    return noutput_items;
}


bool
fmcomms5_source_impl::start()
{
    bool ret;

    ret = rwt_base_block::start();
    if (!ret)
        return ret;

    ret = m_source->start(true, true, true, true);
    if (!ret)
        return ret;

    return ret;
}


bool
fmcomms5_source_impl::stop()
{
    m_source->stop();
    rwt_base_block::stop();
    free_volk_buffers();
    return true;
}


} /* namespace rwt */
} /* namespace gr */
