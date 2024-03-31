#pragma once

#include "loader.hpp"

//
// klhk.sys helpers.
//
namespace klhk
{
	bool load    ( );
	void cleanup ( bool delete_service );
}