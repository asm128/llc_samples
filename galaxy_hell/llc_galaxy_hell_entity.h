#include "llc_enum.h"
#include "llc_apod_serialize.h"

#ifndef LLC_GALAXY_HELL_ENTITY_H
#define LLC_GALAXY_HELL_ENTITY_H

namespace ghg 
{
	usns llc;
#pragma pack(push, 1)
	struct SGHEntity {
		s2_t						Parent					= -1;
		s2_t						Geometry				= -1;
		s2_t						Transform				= -1;
		s2_t						Image					= -1;
		s2_t						Body					= -1;
	};
#pragma pack(pop)

	struct SEntitySystem {
		::llc::apod<::ghg::SGHEntity>	Entities				= {};
		::llc::aobj<::llc::au2_t>		EntityChildren			= {};

		llc::err_t				Create					(const ::ghg::SGHEntity & newEntity, ::llc::vcu2_t entityChildren = {})	{ EntityChildren.push_back(entityChildren); return Entities.push_back(newEntity); }
		llc::err_t				Create					()																		{ Create({}); }

		llc::err_t				Save					(::llc::au0_t & output) const { 
			if_fail_fe(::llc::saveView(output, Entities));
			for(u2_t iEntity = 0; iEntity < Entities.size(); ++iEntity) 
				if_fail_fef(::llc::saveView(output, ::llc::vcu2_t{EntityChildren[iEntity]}), "iEntity: %i", iEntity);
			info_printf("Saved %s, %i", "Entities"					, EntityChildren.size());
			return 0; 
		}

		llc::err_t				Load					(::llc::vcu0_t & input) { 
			if_fail_fe(::llc::loadView(input, Entities));
			if_fail_fef(EntityChildren.resize(Entities.size()), "size: %i", Entities.size());
			for(u2_t iEntity = 0; iEntity < Entities.size(); ++iEntity)
				if_fail_fef(::llc::loadView(input, EntityChildren[iEntity]), "iEntity: %i", iEntity);

			return 0; 
		}		
		// 11 6688 8367
	};
}

#endif // LLC_GALAXY_HELL_ENTITY_H