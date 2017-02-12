 #ifndef _TS_COMMON_H
#define _TS_COMMON_H

#include "pgx_ts_common_def.h"
#include <stdint.h>
#include <vector>
#include <string>
using namespace std;
#ifdef _WIN32
typedef unsigned char   	u8;
typedef unsigned int    	u32;
typedef unsigned __int64    u64;
#else
typedef uint8_t   	u8;
typedef uint32_t  	u32;
typedef uint64_t    u64;
#endif

void AppendBits(u8 *buff, int &nOffset, int length, u8 data);

/*****************************************************************
 *
 * PCR
 *
 * 6 byte
 *****************************************************************/

typedef struct LIB_TINY_TS_API _ts_pcr_t
{
	u64 program_clock_reference_base      : 33;
	u32 reserved                          : 6;
	u32 program_clock_reference_extension : 9;
}ts_pcr_t;

typedef struct LIB_TINY_TS_API _ts_adaption_t
{
	enum{
		TS_ADAPTION_ZERO								   = 0X00,
		TS_ADAPTION_DISCONTINUITY_INDICATOR_1			   = 0X80,
		TS_ADAPTION_RANDOM_ACCESS_INDICATOR_1			   = 0X40,
		TS_ADAPTION_ELEMENTARY_STREAM_PRIORITY_INDICATOR_1 = 0X20,
		TS_ADAPTION_PCR_FLAG_1							   = 0X10,
		TS_ADAPTION_OPCR_FLAG_1							   = 0X08,
		TS_ADAPTION_SPLICING_POINT_FLAG_1				   = 0X04,
		TS_ADAPTION_TRANSPORT_PRIVATE_DATA_FLAG_1		   = 0X02,
		TS_ADAPTION_FIELD_EXTENSION_FLAG_1				   = 0X01
	};

	u8 adaption_len;
	u8 adaption_flags;
	u8 *adaption_data;

	_ts_adaption_t()
	{
		Init();
	}

	void Init()
	{
		adaption_len   = 0x00;
		adaption_flags = TS_ADAPTION_ZERO;
		adaption_data  = NULL;
	}
}ts_adaption_t;

/*****************************************************************
 *
 * TS-Header
 *
 * 4 bytes
 ******************************************************************/

typedef struct LIB_TINY_TS_API _ts_header
{
	u32 sync_byte:8;          //同步字 0x47
	u32 transport_error_indicator:1;   //错误标志，0 无错误，1 有错误
	u32 payload_unit_start_indicator:1;//pes起始标记
	u32 transport_priority:1;          //传输优先级
	u32 pid:13;                        //PID号
	u32 transport_scrambling_control:2; //加密标记 ，00非加密
	u32 adaption_field_control:2;       //附加区域控制，11 附加区域+负载，01 只有负载， 10 只有附加区域， 00 无效
	u32 continuity_counter:4;           //同一pid的ts流计数，0~15，循环反复
	u32 payload_len;
	u8  *payload_data;

	_ts_header()
	{
		continuity_counter = 0;
		Init();
	};

	void Init()
	{
		sync_byte = 0x47;
		transport_error_indicator = 0x00;
		payload_unit_start_indicator = 0x01;
		transport_priority = 0x00;
		pid = 0x00;
		transport_scrambling_control = 0x00;
		adaption_field_control = 0x01;
		//continuity_counter = 0x00;
		payload_len = 0;
		payload_data = NULL;
	}

	void ContinuityCounterACC()
	{
		if (continuity_counter>=15)
		{
			continuity_counter = 0;
		}
		else
		{
			continuity_counter++;
		}
	}

}ts_header;

/*****************************************************************
*
*SDT-HEADER
*
*
******************************************************************/


typedef struct LIB_TINY_TS_API _ts_sdt_service_descriptor_t
{
	u32 descriptor_tag_8:8;
	u32 descriptor_length_8:8;
	u32 service_type_8:8;
	u32 service_provider_name_length_8:8;
	vector<u8> provider_name;
	u32 service_name_length_8:8;
	vector<u8> service_name;

	_ts_sdt_service_descriptor_t()
	{
		this->Initialize();
	}

	void Initialize()
	{
		descriptor_tag_8 = 0x48;
		descriptor_length_8 = 3;
		service_type_8 = 0x01;
		service_provider_name_length_8 = 0;
		provider_name.clear();
		service_name_length_8 = 0;
		service_name.clear();
	}

	void SetProviderName(string strProviderName)
	{
		service_provider_name_length_8 = strProviderName.length();
		char const *szProviderName = strProviderName.c_str();

		for (int i=0; i<service_provider_name_length_8; ++i)
		{
			provider_name.push_back(szProviderName[i]);
		}

		descriptor_length_8 += service_provider_name_length_8;
	}

	void SetServiceName(string strServiceName)
	{
		service_name_length_8 = strServiceName.length();
		char const *szServiceName = strServiceName.c_str();
		for (int i=0; i<strServiceName.length(); ++i)
		{
			service_name.push_back(szServiceName[i]);
		}

		descriptor_length_8 += service_name_length_8;
	}

	int GetSize()
	{
		int nSize = 2+descriptor_length_8;

		return nSize;
	}
}ts_sdt_service_descriptor_t;

typedef struct LIB_TINY_TS_API _ts_sdt_service_t
{
	u32 service_id_16:16;
	u32 reserved_future_use_6:6;
	u32 EIT_schedule_flag_1:1;
	u32 EIT_present_following_flag_1:1;
	u32 running_status_3:3;
	u32 freed_CA_mode_1:1;
	u32 descriptors_loop_length_12:12;
	vector<ts_sdt_service_descriptor_t*> descriptors;

	_ts_sdt_service_t()
	{
		this->Initialize();
	}

	void Initialize()
	{
		service_id_16 = 0x01;
		reserved_future_use_6 = 0;
		EIT_schedule_flag_1 = 0;
		EIT_present_following_flag_1 = 0;
		running_status_3 = 0x04;
		freed_CA_mode_1 = 0;
		descriptors_loop_length_12 = 0;
	}

	void AttachDescriptor(ts_sdt_service_descriptor_t* descriptor)
	{
		descriptors.push_back(descriptor);
		descriptors_loop_length_12 += descriptor->GetSize();
	}

	int GetSize()
	{
		int nSize = 0;
		nSize = 5+descriptors_loop_length_12;

		return nSize;
	}

}ts_sdt_service_t;

typedef struct LIB_TINY_TS_API _ts_sdt_t
{
	u32 table_id_8:8;
	u32 section_syntax_indicator_1:1;
	u32 reserved_future_use0_1:1;
	u32 reserved0_2:2;
	u32 section_length_12:12;
	u32 transport_stream_id_16:16; //    给出TS识别号
	u32 reserved1_2:2;
	u32 version_number_5:5;
	u32 current_next_indicator_1:1;
	u32 section_number_8:8;
	u32 last_section_number_8:8;
	u32 original_nerwork_id_16:16;
	u32 reserved_future_use1_8:8;
	vector<ts_sdt_service_t*> service;
	u32 crc_32:32;

	_ts_sdt_t()
	{
		this->Initialize();
	}

	void Initialize()
	{
		table_id_8 = 0x42; //0x42 0x46
		section_syntax_indicator_1 = 0x01;
		reserved_future_use0_1 = 0;
		reserved0_2 = 0;
		section_length_12 = 0;
		transport_stream_id_16 = 0x01; //    给出TS识别号
		reserved1_2 = 0;
		version_number_5 = 0;
		current_next_indicator_1 = 0x01;
		section_number_8 = 0;
		last_section_number_8 = 0;
		original_nerwork_id_16 = 0x01;
		reserved_future_use1_8 = 0;

		crc_32 = 0;
	};

	void AttachService(ts_sdt_service_t* s)
	{
		service.push_back(s);
		section_length_12 += 12+s->GetSize();
	}

}ts_sdt_t;


/*****************************************************************
 * PAT-HEADER
 * 
 *
 *
 *****************************************************************/
typedef struct LIB_TINY_TS_API _ts_pat_program_t
{
	enum
	{
		PROGRAM_NUM_PMT = 0X0001,
		PROGRAM_NUM_NIT = 0X0000
	};
	u32 program_number   :  16;  //节目号 , 0x0000 NIT, 0x0001 PMT
	u32 reserved         :  3;
	u32 program_pid      :  13; // 节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个

	_ts_pat_program_t()
	{
		Init();
	}

	void Init()
	{
		program_number = 0x0001;
		reserved = 0xff;
		program_pid = 0x42;
	}

}ts_pat_program_t;

typedef struct LIB_TINY_TS_API _ts_pat_t
{
	u32 table_id                     : 8; //固定为0x00 ，标志是该表是PAT表
	u32 section_syntax_indicator     : 1; //段语法标志位，固定为1
	u32 zero                         : 1; //0
	u32 reserved_1                   : 2; // 保留位
	u32 section_length               : 12; //表示从下一个字段开始到CRC32(含)之间有用的字节数
	u32 transport_stream_id          : 16; //该传输流的ID，区别于一个网络中其它多路复用的流
	u32 reserved_2                   : 2;// 保留位
	u32 version_number               : 5; //范围0-31，表示PAT的版本号
	u32 current_next_indicator       : 1; //发送的PAT是当前有效还是下一个PAT有效,1当前可用，0下张表可用
	u32 section_number               : 8; //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	u32 last_section_number          : 8;  //最后一个分段的号码
	
	vector<ts_pat_program_t*> program;
	u32 CRC_32                       : 32;  //CRC32校验码

	_ts_pat_t()
	{
		program.clear();
		Init();
	};

	virtual ~_ts_pat_t()
	{
	}

	void Init()
	{
		table_id = 0x00;
		section_syntax_indicator = 1;
		zero = 0;
		reserved_1 = 0xff;
		section_length = 0;
		reserved_2 = 0xff;
		version_number = 0x00;
		current_next_indicator = 0x01;
		section_number = 0x00;
		last_section_number = 0x00;
		
		CRC_32 = 0x00000000;
	};

} ts_pat_t; 


/****************************************************************
 * PMT-HEADER
 *
 *
 *
 *****************************************************************/
typedef struct LIB_TINY_TS_API _ts_pmt_stream_t
{
	enum{
		STREAM_TYPE_H264 = 0x1b,
		STREAM_TYPE_H265 = 0x24,
		STREAM_TYPE_AAC  = 0x0f
	};

	u32 stream_type                       : 8; //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	u32 reserved_3                        : 3;
	u32 elementary_PID                    : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素
	u32 reserved_4						  : 4;
	u32 ES_info_length                    : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	vector<u8> descriptor;

	_ts_pmt_stream_t()
	{
		Init();
	};

	void Init()
	{
		stream_type = 0x00;
		reserved_3 = 0xff;
		elementary_PID = 0x00;
		reserved_4 = 0xff;
		ES_info_length = 0x00;
		descriptor.clear();
	};

}ts_pmt_stream_t; 

typedef struct LIB_TINY_TS_API _ts_pmt_t
{
	u32 table_id                          : 8; //固定为0x02, 表示PMT表
	u32 section_syntax_indicator          : 1; //固定为0x01
	u32 zero                              : 1; //0x01
	u32 reserved_1                        : 2; //0x03
	u32 section_length                    : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。
	u32 program_number                    : 16;// 指出该节目对应于可应用的Program map PID
	u32 reserved_2                        : 2; //0x03
	u32 version_number                    : 5; //指出TS流中Program map section的版本号
	u32 current_next_indicator            : 1; //当该位置1时，当前传送的Program map section可用；
											   //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
	u32 section_number                    : 8; //固定为0x00
	u32 last_section_number               : 8; //固定为0x00
	u32 reserved_3                        : 3; //0x07
	u32 PCR_PID                           : 13;//指明TS包的PID值，该TS包含有PCR域，
											   //该PCR值对应于由节目号指定的对应节目。
											   //如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。

	u32 reserved_4                        : 4;  //预留为0x0F
	u32 program_info_length               : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
	vector<u8>  descriptor;

	vector<ts_pmt_stream_t*> PMT_Stream;        //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	u32 CRC_32                            : 32; 

	_ts_pmt_t()
	{
		PMT_Stream.clear();

		table_id                           = 0x02;
		section_syntax_indicator           = 0x01;
		zero                               = 0x00;
		reserved_1                         = 0xff;
		section_length                     = 0x00;
		program_number                     = 0x01;
		reserved_2                         = 0xff;
		version_number                     = 0x00;
		current_next_indicator             = 0x01;

		section_number                     = 0x00;
		last_section_number                = 0x00;
		reserved_3                         = 0xff;
		PCR_PID                            = 0x1fff;

		reserved_4                         = 0xff;
		program_info_length                = 0x00;
		descriptor.clear();


		CRC_32                             = 0x00;

		Init();
		return;
	};

	virtual ~_ts_pmt_t()
	{
	}
	void Init()
	{
		table_id                           = 0x02;
		section_syntax_indicator           = 0x01;
		zero                               = 0x00;
		reserved_1                         = 0xff;
		section_length                     = 0x00;
		program_number                     = 0x01;
		reserved_2                         = 0xff;
		version_number                     = 0x00;
		current_next_indicator             = 0x01;
		 
		section_number                     = 0x00;
		last_section_number                = 0x00;
		reserved_3                         = 0xff;
		
		reserved_4                         = 0xff;
		program_info_length                = 0x00;
		descriptor.clear();
				
		CRC_32                             = 0x00;
	};

}ts_pmt_t;


/****************************************************************
 * PES-HEADER
 *
 *
 *
 *****************************************************************/
#define SIZE_OF_AUD_LENGTH 6
extern const u8 g_AUD[SIZE_OF_AUD_LENGTH]; 

typedef struct LIB_TINY_TS_API _pes_header_t
{
	u8   prefix[3];
	u32  stream_id:8;
	u32  pes_length:16;
	u32  reserved_2:2;
	u32  pes_optional_flags:14;

	enum{
		PES_OPT_ZERO				   = 0X0000,
		PES_OPT_SCRAMBLING_CONTROL_1_0 = 0X2000,
		PES_OPT_SCRAMBLING_CONTROL_0_1 = 0X1000,
		PES_OPT_SCRAMBLING_CONTROL_1_1 = 0X3000,

		PES_OPT_PRIORITY_1				   = 0X0800,
		PES_OPT_DATA_ALIGNMENT_INDICATOR_1 = 0X0400,
		PES_OPT_COPYRIGHT_1				   = 0X0200,
		PES_OPT_ORIGINAL_OR_COPY_1		   = 0X0100,
		PES_OPT_PTS_DTS_FLAGS_PTS_10	   = 0X0080,
		PES_OPT_PTS_DTS_FLAGS_DTS_01	   = 0X0040,
		PES_OPT_ESCR_FLAG_1				   = 0X0020,
		PES_OPT_ES_RATE_FLAG			   = 0X0010,
		PES_OPT_DSM_TRICK_MOD_FLAG		   = 0X0008,
		PES_OPT_ADDITIONAL_COPY_INFO_FLAG  = 0X0004,
		PES_OPT_PES_CRC_FLAG			   = 0X0002,
		PES_OPT_PES_EXTENSION_FLAG		   = 0X0001
	};

	u32  pes_header_length:8;
	
	_pes_header_t()
	{
		Init();
	};

	void Init()
	{
		prefix[0] = 0x00;
		prefix[1] = 0x00;
		prefix[2] = 0x01;

		pes_length = 0x00;
		reserved_2 = 0x02;
		pes_optional_flags = PES_OPT_ZERO;
		
		pes_header_length = 0x00;
	};

}pes_header_t;

static u32 crc32table[256] = {   
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,   
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,   
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,   
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,   
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,   
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,   
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,   
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,   
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,   
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,   
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,   
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,   
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,   
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,   
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,   
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,   
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,   
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,   
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,   
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,   
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,   
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,   
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,   
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,   
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,   
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,   
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,   
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,   
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,   
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,   
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,   
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,   
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,   
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,   
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,   
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,   
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,   
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,   
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,   
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,   
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,   
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,   
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

u32 CRC32(const u8 *data, int len);


#endif
