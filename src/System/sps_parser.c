/**
 * @file sps_parser.c
 * @author Ben
 * @date  23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 * @TODO code refactoring.
 */
/* ******* INCLUDE ******* */
#include "brik_api.h"
#include "brik_utils.h"

/* ******* GLOBAL VARIABLE ******* */
static const unsigned char * m_pStart;
static unsigned short        m_nLength;
static int                   m_nCurrentBit;

/* ******* STATIC FUNCTIONS ******* */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static unsigned int ReadBit(void)
{
    assert(m_nCurrentBit <= m_nLength * 8);
    int nIndex = m_nCurrentBit / 8;
    int nOffset = m_nCurrentBit % 8 + 1;

    m_nCurrentBit++;

    return (m_pStart[nIndex] >> (8-nOffset)) & 0x01;
}

static unsigned int ReadBits(int n)
{
    int r = 0;
    int i = 0;

    for (i = 0; i < n; i++)
    {
        r |= ( ReadBit() << ( n - i - 1 ) );
    }

    return r;
}

static unsigned int ReadExponentialGolombCode(void)
{
    int r = 0;
    int i = 0;

    while( (ReadBit() == 0) && (i < 32) )
    {
        i++;
    }

    r = ReadBits(i);
    r += (1 << i) - 1;

    return r;
}

static unsigned int ReadSE(void)
{
    int r = ReadExponentialGolombCode();

    if (r & 0x01)
    {
        r = (r+1)/2;
    }
    else
    {
        r = -(r/2);
    }

    return r;
}

void sps_parse(const unsigned char * pStart, unsigned short nLen, int32_t *frame_width, int32_t *frame_height)
{
    m_pStart      = pStart;
    m_nLength     = nLen;
    m_nCurrentBit = 0;

    int frame_crop_left_offset   = 0;
    int frame_crop_right_offset  = 0;
    int frame_crop_top_offset    = 0;
    int frame_crop_bottom_offset = 0;

    int crop_unit_x = 0;
    int crop_unit_y = 0;

    int chroma_format_idc = 0;

    int profile_idc = ReadBits(8);
    int constraint_set0_flag = ReadBit();
    int constraint_set1_flag = ReadBit();
    int constraint_set2_flag = ReadBit();
    int constraint_set3_flag = ReadBit();
    int constraint_set4_flag = ReadBit();
    int constraint_set5_flag = ReadBit();
    int reserved_zero_2bits  = ReadBits(2);
    int level_idc = ReadBits(8);
    int seq_parameter_set_id = ReadExponentialGolombCode();

    UNUSED(constraint_set0_flag);
    UNUSED(constraint_set1_flag);
    UNUSED(constraint_set2_flag);
    UNUSED(constraint_set3_flag);
    UNUSED(constraint_set4_flag);
    UNUSED(constraint_set5_flag);
    UNUSED(reserved_zero_2bits);
    UNUSED(level_idc);
    UNUSED(seq_parameter_set_id);

    if( profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 244 ||
        profile_idc == 44  || profile_idc == 83  ||
        profile_idc == 86  || profile_idc == 118    )
    {
        chroma_format_idc = ReadExponentialGolombCode();

        if(chroma_format_idc == 3)
        {
            int residual_colour_transform_flag = ReadBit();
            UNUSED(residual_colour_transform_flag);
        }

        int bit_depth_luma_minus8 = ReadExponentialGolombCode();
        int bit_depth_chroma_minus8 = ReadExponentialGolombCode();
        int qpprime_y_zero_transform_bypass_flag = ReadBit();
        int seq_scaling_matrix_present_flag = ReadBit();
        UNUSED(bit_depth_luma_minus8);
        UNUSED(bit_depth_chroma_minus8);
        UNUSED(seq_scaling_matrix_present_flag);
        UNUSED(qpprime_y_zero_transform_bypass_flag);

        if(seq_scaling_matrix_present_flag)
        {
            int i =0 ;
            for(i = 0; i < 8; i++)
            {
                int seq_scaling_list_present_flag = ReadBit();
                UNUSED(seq_scaling_list_present_flag);

                if (seq_scaling_list_present_flag)
                {
                    int sizeOfScalingList = (i < 6) ? 16 : 64;
                    int lastScale = 8;
                    int nextScale = 8;
                    int j=0;

                    for(j = 0; j < sizeOfScalingList; j++)
                    {
                        if(nextScale != 0)
                        {
                            int delta_scale = ReadSE();
                            nextScale = (lastScale + delta_scale + 256) % 256;
                        }

                        lastScale = (nextScale == 0) ? lastScale : nextScale;
                    }
                }
            }
        }
    }

    int log2_max_frame_num_minus4 = ReadExponentialGolombCode();
    int pic_order_cnt_type = ReadExponentialGolombCode();
    UNUSED(log2_max_frame_num_minus4);
    UNUSED(pic_order_cnt_type);

    if( pic_order_cnt_type == 0 )
    {
        int log2_max_pic_order_cnt_lsb_minus4 = ReadExponentialGolombCode();
        UNUSED(log2_max_pic_order_cnt_lsb_minus4);
    }
    else if( pic_order_cnt_type == 1 )
    {
        int delta_pic_order_always_zero_flag = ReadBit();
        int offset_for_non_ref_pic = ReadSE();
        int offset_for_top_to_bottom_field = ReadSE();
        int num_ref_frames_in_pic_order_cnt_cycle = ReadExponentialGolombCode();
        UNUSED(delta_pic_order_always_zero_flag);
        UNUSED(offset_for_non_ref_pic);
        UNUSED(offset_for_top_to_bottom_field);
        UNUSED(num_ref_frames_in_pic_order_cnt_cycle);

        int i = 0;
        for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
        {
            ReadSE();
            //sps->offset_for_ref_frame[ i ] = ReadSE();
        }
    }
    int max_num_ref_frames = ReadExponentialGolombCode();
    int gaps_in_frame_num_value_allowed_flag = ReadBit();
    int pic_width_in_mbs_minus1 = ReadExponentialGolombCode();
    int pic_height_in_map_units_minus1 = ReadExponentialGolombCode();
    int frame_mbs_only_flag = ReadBit();
    UNUSED(max_num_ref_frames);
    UNUSED(gaps_in_frame_num_value_allowed_flag);
    UNUSED(pic_width_in_mbs_minus1);
    UNUSED(pic_height_in_map_units_minus1);
    UNUSED(frame_mbs_only_flag);

    if( !frame_mbs_only_flag )
    {
        int mb_adaptive_frame_field_flag = ReadBit();
        UNUSED(mb_adaptive_frame_field_flag);
    }

    int direct_8x8_inference_flag = ReadBit();
    int frame_cropping_flag = ReadBit();
    UNUSED(direct_8x8_inference_flag);
    UNUSED(frame_cropping_flag);

    if( frame_cropping_flag )
    {
        frame_crop_left_offset = ReadExponentialGolombCode();
        frame_crop_right_offset = ReadExponentialGolombCode();
        frame_crop_top_offset = ReadExponentialGolombCode();
        frame_crop_bottom_offset = ReadExponentialGolombCode();

        if (chroma_format_idc == 0)
        {
            crop_unit_x = 1;
            crop_unit_y = 2 - frame_mbs_only_flag;
        }
        else
        {
            int sub_width_c = (chroma_format_idc == 3)?1:2;
            int sub_height_c = (chroma_format_idc == 1)?2:1;
            crop_unit_x = sub_width_c;
            crop_unit_y = sub_height_c * ( 2 - frame_mbs_only_flag );
        }
    }

    int vui_parameters_present_flag = ReadBit();
    UNUSED(vui_parameters_present_flag);

    pStart++;

    int Width =  ((pic_width_in_mbs_minus1 +1)*16) - ((frame_crop_right_offset + frame_crop_left_offset) * crop_unit_x);
    int Height = ((2 - frame_mbs_only_flag) *  (pic_height_in_map_units_minus1 +1) * 16) - ((frame_crop_bottom_offset + frame_crop_top_offset)* crop_unit_y);

    printf("\n\nSPS frame dimension: = %d x %d\n\n",Width, Height);

    *frame_width = Width;
    *frame_height = Height;

}
