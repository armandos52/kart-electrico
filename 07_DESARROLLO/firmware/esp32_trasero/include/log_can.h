#pragma once
#include "telemetria.h"
#include <mcp_can.h>
// Snapshot every 200 ms, one best-effort fragment per service call, no wait.
void log_can_snapshot(const MuestraKart& m,uint8_t profile);
void log_can_service(MCP_CAN& can,uint32_t now,bool ready);
void telemetria_log_stats(uint32_t* loop_us,uint32_t* cost_us,uint32_t* dropped,uint8_t* clients,uint8_t* ap);
