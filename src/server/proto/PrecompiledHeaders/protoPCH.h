// PrecompiledHeader for the `proto` static library.
//
// The Legion-era BNet protobuf descriptors are committed as pre-generated
// .pb.h/.pb.cc files under src/server/proto/Client/.  Keep this PCH in sync
// with the descriptor set so missing files are detected early when the proto
// target is compiled.

#include "Login.pb.h"
#include "RealmList.pb.h"

// BNet service descriptors used by bnetserver, shared, and game/Services.
#include "Client/account_service.pb.h"
#include "Client/authentication_service.pb.h"
#include "Client/challenge_service.pb.h"
#include "Client/channel_service.pb.h"
#include "Client/connection_service.pb.h"
#include "Client/friends_service.pb.h"
#include "Client/game_utilities_service.pb.h"
#include "Client/notification_types.pb.h"
#include "Client/presence_service.pb.h"
#include "Client/profanity_filter_config.pb.h"
#include "Client/report_service.pb.h"
#include "Client/resource_service.pb.h"
#include "Client/user_manager_service.pb.h"

// Common BNet type descriptors referenced by the service descriptors.
#include "Client/account_types.pb.h"
#include "Client/attribute_types.pb.h"
#include "Client/channel_types.pb.h"
#include "Client/content_handle_types.pb.h"
#include "Client/entity_types.pb.h"
#include "Client/friends_types.pb.h"
#include "Client/game_utilities_types.pb.h"
#include "Client/invitation_types.pb.h"
#include "Client/presence_types.pb.h"
#include "Client/report_types.pb.h"
#include "Client/resource_service.pb.h"
#include "Client/role_types.pb.h"
#include "Client/rpc_config.pb.h"
#include "Client/rpc_types.pb.h"
#include "Client/user_manager_types.pb.h"
#include "Client/client/v1/channel_id.pb.h"
#include "Client/global_extensions/field_options.pb.h"
#include "Client/global_extensions/method_options.pb.h"
#include "Client/global_extensions/service_options.pb.h"

#include "ServiceBase.h"
#include "Debugging/Errors.h"
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
