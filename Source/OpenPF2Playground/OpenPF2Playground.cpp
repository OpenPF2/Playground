// OpenPF2 for UE Game Logic, Copyright 2021-2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2Playground.h"

#include <Modules/ModuleManager.h>

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, OpenPF2Playground, "OpenPF2Playground");

// =====================================================================================================================
// Logging Definitions
// =====================================================================================================================
DEFINE_LOG_CATEGORY(LogPf2Playground);
DEFINE_LOG_CATEGORY(LogPf2PlaygroundInput);
