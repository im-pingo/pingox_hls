#include <memory.h>
#include "pgx_sdt_packet.h"

CSDTPacket::CSDTPacket(int nPid/*=0x11*/):m_nPid(nPid)
{

}

CSDTPacket::~CSDTPacket()
{
	for (vector<ts_sdt_service_t*>::iterator it=m_sdt.service.begin(); it!=m_sdt.service.end(); ++it)
	{
		ts_sdt_service_t *pService = (*it);
		if (pService)
		{
			for (vector<ts_sdt_service_descriptor_t*>::iterator itDes=pService->descriptors.begin();
				itDes != pService->descriptors.end(); itDes++)
			{
				if (*itDes)
				{
					delete *itDes;
				}
			}

			pService->descriptors.clear();
			delete pService;
		}
	}

	m_sdt.service.clear();
}

void CSDTPacket::Initialize()
{
	memset(m_SDTBuff, 0x00, SIZE_OF_MAX_SDT_PACKET);
	m_nBuffOffset = 0;
	m_sdt.Initialize();
}

void CSDTPacket::SetDescriptor(const char *szProviderName, const char *szServiceName)
{
	ts_sdt_service_descriptor_t *descriptor = new ts_sdt_service_descriptor_t();
	ts_sdt_service_t *service = new ts_sdt_service_t();

	descriptor->Initialize();
	descriptor->SetProviderName(szProviderName);
	descriptor->SetServiceName(szServiceName);

	service->Initialize();
	service->AttachDescriptor(descriptor);

	m_sdt.AttachService(service);
}

int CSDTPacket::FillPATPacket()
{
	//this->Initialize();

	memset(m_SDTBuff, 0x00, SIZE_OF_MAX_SDT_PACKET);
	m_nBuffOffset = 0;

	AppendBits(m_SDTBuff, m_nBuffOffset, 8, 0x00);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.table_id_8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 1, m_sdt.section_syntax_indicator_1);
	AppendBits(m_SDTBuff, m_nBuffOffset, 1, m_sdt.reserved_future_use0_1);
	AppendBits(m_SDTBuff, m_nBuffOffset, 2, m_sdt.reserved0_2);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.section_length_12>>4);
	AppendBits(m_SDTBuff, m_nBuffOffset, 4, m_sdt.section_length_12);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.transport_stream_id_16>>8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.transport_stream_id_16);
	AppendBits(m_SDTBuff, m_nBuffOffset, 2, m_sdt.reserved1_2);
	AppendBits(m_SDTBuff, m_nBuffOffset, 5, m_sdt.version_number_5);
	AppendBits(m_SDTBuff, m_nBuffOffset, 1, m_sdt.current_next_indicator_1);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.section_number_8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.last_section_number_8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.original_nerwork_id_16>>8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.original_nerwork_id_16);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.reserved_future_use1_8);

	for (vector<ts_sdt_service_t*>::iterator it=m_sdt.service.begin(); it!=m_sdt.service.end(); ++it)
	{
		ts_sdt_service_t* pserver = *it;
		AppendBits(m_SDTBuff, m_nBuffOffset, 8, pserver->service_id_16>>8);
		AppendBits(m_SDTBuff, m_nBuffOffset, 8, pserver->service_id_16);
		AppendBits(m_SDTBuff, m_nBuffOffset, 6, pserver->reserved_future_use_6);
		AppendBits(m_SDTBuff, m_nBuffOffset, 1, pserver->EIT_schedule_flag_1);
		AppendBits(m_SDTBuff, m_nBuffOffset, 1, pserver->EIT_present_following_flag_1);
		AppendBits(m_SDTBuff, m_nBuffOffset, 3, pserver->running_status_3);
		AppendBits(m_SDTBuff, m_nBuffOffset, 1, pserver->freed_CA_mode_1);
		AppendBits(m_SDTBuff, m_nBuffOffset, 8, pserver->descriptors_loop_length_12>>4);
		AppendBits(m_SDTBuff, m_nBuffOffset, 4, pserver->descriptors_loop_length_12);
		
		for (vector<ts_sdt_service_descriptor_t*>::iterator itDes = pserver->descriptors.begin(); itDes!=pserver->descriptors.end(); ++itDes)
		{
			ts_sdt_service_descriptor_t* pDes = *itDes;
			AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->descriptor_tag_8);
			AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->descriptor_length_8);
			AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->service_type_8);

			AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->service_provider_name_length_8);
			for(int i=0; i<pDes->provider_name.size(); ++i)
			{
				AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->provider_name[i]);
			}

			AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->service_name_length_8);
			
			for (int j=0; j<pDes->service_name.size(); ++j)
			{
				AppendBits(m_SDTBuff, m_nBuffOffset, 8, pDes->service_name[j]);
			}
		}
	}

	m_sdt.crc_32 = CRC32(m_SDTBuff+1, (m_nBuffOffset/8)-1);

	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.crc_32>>24);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.crc_32>>16);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.crc_32>>8);
	AppendBits(m_SDTBuff, m_nBuffOffset, 8, m_sdt.crc_32);

	memset(m_SDTBuff+(m_nBuffOffset/8), 0xff, SIZE_OF_MAX_SDT_PACKET-(m_nBuffOffset/8));

	return /*m_nBuffOffset/8;*/SIZE_OF_MAX_SDT_PACKET;
}

u8 *CSDTPacket::GetSDTBuff()
{
	return m_SDTBuff;
}

int CSDTPacket::GetSDTPid()
{
	return m_nPid;
}
