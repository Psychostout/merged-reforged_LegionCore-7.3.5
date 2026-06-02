// PrecompiledHeader for the `proto` static library.
//
// NOTE — KNOWN LIMITATION:
//   The 13 BNet service .pb.h files (account_service, authentication_service,
//   challenge_service, channel_service, connection_service, friends_service,
//   game_utilities_service, notification_types, presence_service,
//   profanity_filter_config, report_service, resource_service,
//   user_manager_service) were NOT committed to this repository — neither as
//   pre-generated .pb.h/.pb.cc nor as .proto source files. They are still
//   referenced by:
//     * src/server/bnetserver/Services/AccountService.h
//     * src/server/bnetserver/Services/AuthenticationService.h
//     * src/server/bnetserver/Services/ConnectionService.h
//     * src/server/game/Services/WorldserverService.h
//     * src/server/game/Services/WorldserverServiceDispatcher.h
//     * src/server/shared/Realm/RealmList.cpp
//
//   We investigated importing them from upstream TrinityCore master but
//   TC master has restructured the proto tree into api/{client,common}/v1/
//   subdirectories with hundreds of files; the descriptor schemas have
//   evolved enough (TC master targets Dragonflight/11.x) that they are NOT
//   ABI-compatible with Legion 7.3.5's expectations. They need to be
//   regenerated specifically for Legion 7.3.5 from a period-matched proto
//   source set.
//
//   This PCH only references headers that currently exist in the tree, so
//   `proto`, `common`, `database`, and (in static-scripts mode) all the
//   `scripts/*` subdirectories build cleanly.
//   `shared`, `bnetserver`, and `worldserver` need the missing BNet
//   descriptors before they will fully link — see README.md.

#include "Login.pb.h"
#include "RealmList.pb.h"

#include "ServiceBase.h"
#include "Debugging/Errors.h"
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
