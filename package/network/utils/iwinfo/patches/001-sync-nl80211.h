--- a/api/nl80211.h
+++ b/api/nl80211.h
@@ -29,6 +29,13 @@
 
 #define NL80211_GENL_NAME "nl80211"
 
+#define NL80211_MULTICAST_GROUP_CONFIG		"config"
+#define NL80211_MULTICAST_GROUP_SCAN		"scan"
+#define NL80211_MULTICAST_GROUP_REG		"regulatory"
+#define NL80211_MULTICAST_GROUP_MLME		"mlme"
+#define NL80211_MULTICAST_GROUP_VENDOR		"vendor"
+#define NL80211_MULTICAST_GROUP_TESTMODE	"testmode"
+
 /**
  * DOC: Station handling
  *
@@ -173,8 +180,8 @@
  *	%NL80211_ATTR_WIPHY and %NL80211_ATTR_WIPHY_NAME.
  *
  * @NL80211_CMD_GET_INTERFACE: Request an interface's configuration;
- *	either a dump request on a %NL80211_ATTR_WIPHY or a specific get
- *	on an %NL80211_ATTR_IFINDEX is supported.
+ *	either a dump request for all interfaces or a specific get with a
+ *	single %NL80211_ATTR_IFINDEX is supported.
  * @NL80211_CMD_SET_INTERFACE: Set type of a virtual interface, requires
  *	%NL80211_ATTR_IFINDEX and %NL80211_ATTR_IFTYPE.
  * @NL80211_CMD_NEW_INTERFACE: Newly created virtual interface or response
@@ -227,7 +234,11 @@
  *	the interface identified by %NL80211_ATTR_IFINDEX.
  * @NL80211_CMD_DEL_STATION: Remove a station identified by %NL80211_ATTR_MAC
  *	or, if no MAC address given, all stations, on the interface identified
- *	by %NL80211_ATTR_IFINDEX.
+ *	by %NL80211_ATTR_IFINDEX. %NL80211_ATTR_MGMT_SUBTYPE and
+ *	%NL80211_ATTR_REASON_CODE can optionally be used to specify which type
+ *	of disconnection indication should be sent to the station
+ *	(Deauthentication or Disassociation frame and reason code for that
+ *	frame).
  *
  * @NL80211_CMD_GET_MPATH: Get mesh path attributes for mesh path to
  * 	destination %NL80211_ATTR_MAC on the interface identified by
@@ -248,7 +259,18 @@
  *	%NL80211_ATTR_IFINDEX.
  *
  * @NL80211_CMD_GET_REG: ask the wireless core to send us its currently set
- * 	regulatory domain.
+ *	regulatory domain. If %NL80211_ATTR_WIPHY is specified and the device
+ *	has a private regulatory domain, it will be returned. Otherwise, the
+ *	global regdomain will be returned.
+ *	A device will have a private regulatory domain if it uses the
+ *	regulatory_hint() API. Even when a private regdomain is used the channel
+ *	information will still be mended according to further hints from
+ *	the regulatory core to help with compliance. A dump version of this API
+ *	is now available which will returns the global regdomain as well as
+ *	all private regdomains of present wiphys (for those that have it).
+ *	If a wiphy is self-managed (%NL80211_ATTR_WIPHY_SELF_MANAGED_REG), then
+ *	its private regdomain is the only valid one for it. The regulatory
+ *	core is not used to help with compliance in this case.
  * @NL80211_CMD_SET_REG: Set current regulatory domain. CRDA sends this command
  *	after being queried by the kernel. CRDA replies by sending a regulatory
  *	domain structure which consists of %NL80211_ATTR_REG_ALPHA set to our
@@ -302,7 +324,9 @@
  *	if passed, define which channels should be scanned; if not
  *	passed, all channels allowed for the current regulatory domain
  *	are used.  Extra IEs can also be passed from the userspace by
- *	using the %NL80211_ATTR_IE attribute.
+ *	using the %NL80211_ATTR_IE attribute.  The first cycle of the
+ *	scheduled scan can be delayed by %NL80211_ATTR_SCHED_SCAN_DELAY
+ *	is supplied.
  * @NL80211_CMD_STOP_SCHED_SCAN: stop a scheduled scan. Returns -ENOENT if
  *	scheduled scan is not running. The caller may assume that as soon
  *	as the call returns, it is safe to start a new scheduled scan again.
@@ -639,7 +663,18 @@
  * @NL80211_CMD_CH_SWITCH_NOTIFY: An AP or GO may decide to switch channels
  *	independently of the userspace SME, send this event indicating
  *	%NL80211_ATTR_IFINDEX is now on %NL80211_ATTR_WIPHY_FREQ and the
- *	attributes determining channel width.
+ *	attributes determining channel width.  This indication may also be
+ *	sent when a remotely-initiated switch (e.g., when a STA receives a CSA
+ *	from the remote AP) is completed;
+ *
+ * @NL80211_CMD_CH_SWITCH_STARTED_NOTIFY: Notify that a channel switch
+ *	has been started on an interface, regardless of the initiator
+ *	(ie. whether it was requested from a remote device or
+ *	initiated on our own).  It indicates that
+ *	%NL80211_ATTR_IFINDEX will be on %NL80211_ATTR_WIPHY_FREQ
+ *	after %NL80211_ATTR_CH_SWITCH_COUNT TBTT's.  The userspace may
+ *	decide to react to this indication by requesting other
+ *	interfaces to change channel as well.
  *
  * @NL80211_CMD_START_P2P_DEVICE: Start the given P2P Device, identified by
  *	its %NL80211_ATTR_WDEV identifier. It must have been created with
@@ -722,6 +757,47 @@
  *	QoS mapping is relevant for IP packets, it is only valid during an
  *	association. This is cleared on disassociation and AP restart.
  *
+ * @NL80211_CMD_ADD_TX_TS: Ask the kernel to add a traffic stream for the given
+ *	%NL80211_ATTR_TSID and %NL80211_ATTR_MAC with %NL80211_ATTR_USER_PRIO
+ *	and %NL80211_ATTR_ADMITTED_TIME parameters.
+ *	Note that the action frame handshake with the AP shall be handled by
+ *	userspace via the normal management RX/TX framework, this only sets
+ *	up the TX TS in the driver/device.
+ *	If the admitted time attribute is not added then the request just checks
+ *	if a subsequent setup could be successful, the intent is to use this to
+ *	avoid setting up a session with the AP when local restrictions would
+ *	make that impossible. However, the subsequent "real" setup may still
+ *	fail even if the check was successful.
+ * @NL80211_CMD_DEL_TX_TS: Remove an existing TS with the %NL80211_ATTR_TSID
+ *	and %NL80211_ATTR_MAC parameters. It isn't necessary to call this
+ *	before removing a station entry entirely, or before disassociating
+ *	or similar, cleanup will happen in the driver/device in this case.
+ *
+ * @NL80211_CMD_GET_MPP: Get mesh path attributes for mesh proxy path to
+ *	destination %NL80211_ATTR_MAC on the interface identified by
+ *	%NL80211_ATTR_IFINDEX.
+ *
+ * @NL80211_CMD_JOIN_OCB: Join the OCB network. The center frequency and
+ *	bandwidth of a channel must be given.
+ * @NL80211_CMD_LEAVE_OCB: Leave the OCB network -- no special arguments, the
+ *	network is determined by the network interface.
+ *
+ * @NL80211_CMD_TDLS_CHANNEL_SWITCH: Start channel-switching with a TDLS peer,
+ *	identified by the %NL80211_ATTR_MAC parameter. A target channel is
+ *	provided via %NL80211_ATTR_WIPHY_FREQ and other attributes determining
+ *	channel width/type. The target operating class is given via
+ *	%NL80211_ATTR_OPER_CLASS.
+ *	The driver is responsible for continually initiating channel-switching
+ *	operations and returning to the base channel for communication with the
+ *	AP.
+ * @NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH: Stop channel-switching with a TDLS
+ *	peer given by %NL80211_ATTR_MAC. Both peers must be on the base channel
+ *	when this command completes.
+ *
+ * @NL80211_CMD_WIPHY_REG_CHANGE: Similar to %NL80211_CMD_REG_CHANGE, but used
+ *	as an event to indicate changes for devices with wiphy-specific regdom
+ *	management.
+ *
  * @NL80211_CMD_MAX: highest used command number
  * @__NL80211_CMD_AFTER_LAST: internal use
  */
@@ -893,6 +969,21 @@ enum nl80211_commands {
 
 	NL80211_CMD_SET_QOS_MAP,
 
+	NL80211_CMD_ADD_TX_TS,
+	NL80211_CMD_DEL_TX_TS,
+
+	NL80211_CMD_GET_MPP,
+
+	NL80211_CMD_JOIN_OCB,
+	NL80211_CMD_LEAVE_OCB,
+
+	NL80211_CMD_CH_SWITCH_STARTED_NOTIFY,
+
+	NL80211_CMD_TDLS_CHANNEL_SWITCH,
+	NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH,
+
+	NL80211_CMD_WIPHY_REG_CHANGE,
+
 	/* add new commands above here */
 
 	/* used to define NL80211_CMD_MAX below */
@@ -1587,13 +1678,72 @@ enum nl80211_commands {
  * @NL80211_ATTR_TDLS_PEER_CAPABILITY: flags for TDLS peer capabilities, u32.
  *	As specified in the &enum nl80211_tdls_peer_capability.
  *
- * @NL80211_ATTR_IFACE_SOCKET_OWNER: flag attribute, if set during interface
+ * @NL80211_ATTR_SOCKET_OWNER: Flag attribute, if set during interface
  *	creation then the new interface will be owned by the netlink socket
- *	that created it and will be destroyed when the socket is closed
+ *	that created it and will be destroyed when the socket is closed.
+ *	If set during scheduled scan start then the new scan req will be
+ *	owned by the netlink socket that created it and the scheduled scan will
+ *	be stopped when the socket is closed.
+ *
+ * @NL80211_ATTR_TDLS_INITIATOR: flag attribute indicating the current end is
+ *	the TDLS link initiator.
+ *
+ * @NL80211_ATTR_USE_RRM: flag for indicating whether the current connection
+ *	shall support Radio Resource Measurements (11k). This attribute can be
+ *	used with %NL80211_CMD_ASSOCIATE and %NL80211_CMD_CONNECT requests.
+ *	User space applications are expected to use this flag only if the
+ *	underlying device supports these minimal RRM features:
+ *		%NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES,
+ *		%NL80211_FEATURE_QUIET,
+ *	If this flag is used, driver must add the Power Capabilities IE to the
+ *	association request. In addition, it must also set the RRM capability
+ *	flag in the association request's Capability Info field.
+ *
+ * @NL80211_ATTR_WIPHY_DYN_ACK: flag attribute used to enable ACK timeout
+ *	estimation algorithm (dynack). In order to activate dynack
+ *	%NL80211_FEATURE_ACKTO_ESTIMATION feature flag must be set by lower
+ *	drivers to indicate dynack capability. Dynack is automatically disabled
+ *	setting valid value for coverage class.
+ *
+ * @NL80211_ATTR_TSID: a TSID value (u8 attribute)
+ * @NL80211_ATTR_USER_PRIO: user priority value (u8 attribute)
+ * @NL80211_ATTR_ADMITTED_TIME: admitted time in units of 32 microseconds
+ *	(per second) (u16 attribute)
+ *
+ * @NL80211_ATTR_SMPS_MODE: SMPS mode to use (ap mode). see
+ *	&enum nl80211_smps_mode.
+ *
+ * @NL80211_ATTR_OPER_CLASS: operating class
+ *
+ * @NL80211_ATTR_MAC_MASK: MAC address mask
+ *
+ * @NL80211_ATTR_WIPHY_SELF_MANAGED_REG: flag attribute indicating this device
+ *	is self-managing its regulatory information and any regulatory domain
+ *	obtained from it is coming from the device's wiphy and not the global
+ *	cfg80211 regdomain.
+ *
+ * @NL80211_ATTR_EXT_FEATURES: extended feature flags contained in a byte
+ *	array. The feature flags are identified by their bit index (see &enum
+ *	nl80211_ext_feature_index). The bit index is ordered starting at the
+ *	least-significant bit of the first byte in the array, ie. bit index 0
+ *	is located at bit 0 of byte 0. bit index 25 would be located at bit 1
+ *	of byte 3 (u8 array).
+ *
+ * @NL80211_ATTR_SURVEY_RADIO_STATS: Request overall radio statistics to be
+ *	returned along with other survey data. If set, @NL80211_CMD_GET_SURVEY
+ *	may return a survey entry without a channel indicating global radio
+ *	statistics (only some values are valid and make sense.)
+ *	For devices that don't return such an entry even then, the information
+ *	should be contained in the result as the sum of the respective counters
+ *	over all channels.
+ *
+ * @NL80211_ATTR_SCHED_SCAN_DELAY: delay before a scheduled scan (or a
+ *	WoWLAN net-detect scan) is started, u32 in seconds.
  *
  * @NL80211_ATTR_WIPHY_ANTENNA_GAIN: Configured antenna gain. Used to reduce
  *	transmit power to stay within regulatory limits. u32, dBi.
  *
+ * @NUM_NL80211_ATTR: total number of nl80211_attrs available
  * @NL80211_ATTR_MAX: highest attribute number currently defined
  * @__NL80211_ATTR_AFTER_LAST: internal use
  */
@@ -1929,22 +2079,50 @@ enum nl80211_attrs {
 
 	NL80211_ATTR_TDLS_PEER_CAPABILITY,
 
-	NL80211_ATTR_IFACE_SOCKET_OWNER,
+	NL80211_ATTR_SOCKET_OWNER,
 
 	NL80211_ATTR_CSA_C_OFFSETS_TX,
 	NL80211_ATTR_MAX_CSA_COUNTERS,
 
+	NL80211_ATTR_TDLS_INITIATOR,
+
+	NL80211_ATTR_USE_RRM,
+
+	NL80211_ATTR_WIPHY_DYN_ACK,
+
+	NL80211_ATTR_TSID,
+	NL80211_ATTR_USER_PRIO,
+	NL80211_ATTR_ADMITTED_TIME,
+
+	NL80211_ATTR_SMPS_MODE,
+
+	NL80211_ATTR_OPER_CLASS,
+
+	NL80211_ATTR_MAC_MASK,
+
+	NL80211_ATTR_WIPHY_SELF_MANAGED_REG,
+
+	NL80211_ATTR_EXT_FEATURES,
+
+	NL80211_ATTR_SURVEY_RADIO_STATS,
+
+	NL80211_ATTR_NETNS_FD,
+
+	NL80211_ATTR_SCHED_SCAN_DELAY,
+
 	NL80211_ATTR_WIPHY_ANTENNA_GAIN,
 
 	/* add attributes here, update the policy in nl80211.c */
 
 	__NL80211_ATTR_AFTER_LAST,
+	NUM_NL80211_ATTR = __NL80211_ATTR_AFTER_LAST,
 	NL80211_ATTR_MAX = __NL80211_ATTR_AFTER_LAST - 1
 };
 
 /* source-level API compatibility */
 #define NL80211_ATTR_SCAN_GENERATION NL80211_ATTR_GENERATION
 #define	NL80211_ATTR_MESH_PARAMS NL80211_ATTR_MESH_CONFIG
+#define NL80211_ATTR_IFACE_SOCKET_OWNER NL80211_ATTR_SOCKET_OWNER
 
 /*
  * Allow user space programs to use #ifdef on new attributes by defining them
@@ -1974,7 +2152,7 @@ enum nl80211_attrs {
 
 #define NL80211_MAX_SUPP_RATES			32
 #define NL80211_MAX_SUPP_HT_RATES		77
-#define NL80211_MAX_SUPP_REG_RULES		32
+#define NL80211_MAX_SUPP_REG_RULES		64
 #define NL80211_TKIP_DATA_OFFSET_ENCR_KEY	0
 #define NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY	16
 #define NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY	24
@@ -2010,6 +2188,8 @@ enum nl80211_attrs {
  *	and therefore can't be created in the normal ways, use the
  *	%NL80211_CMD_START_P2P_DEVICE and %NL80211_CMD_STOP_P2P_DEVICE
  *	commands to create and destroy one
+ * @NL80211_IF_TYPE_OCB: Outside Context of a BSS
+ *	This mode corresponds to the MIB variable dot11OCBActivated=true
  * @NL80211_IFTYPE_MAX: highest interface type number currently defined
  * @NUM_NL80211_IFTYPES: number of defined interface types
  *
@@ -2029,6 +2209,7 @@ enum nl80211_iftype {
 	NL80211_IFTYPE_P2P_CLIENT,
 	NL80211_IFTYPE_P2P_GO,
 	NL80211_IFTYPE_P2P_DEVICE,
+	NL80211_IFTYPE_OCB,
 
 	/* keep last */
 	NUM_NL80211_IFTYPES,
@@ -2111,8 +2292,15 @@ struct nl80211_sta_flag_update {
  * @NL80211_RATE_INFO_VHT_MCS: MCS index for VHT (u8)
  * @NL80211_RATE_INFO_VHT_NSS: number of streams in VHT (u8)
  * @NL80211_RATE_INFO_80_MHZ_WIDTH: 80 MHz VHT rate
- * @NL80211_RATE_INFO_80P80_MHZ_WIDTH: 80+80 MHz VHT rate
+ * @NL80211_RATE_INFO_80P80_MHZ_WIDTH: unused - 80+80 is treated the
+ *	same as 160 for purposes of the bitrates
  * @NL80211_RATE_INFO_160_MHZ_WIDTH: 160 MHz VHT rate
+ * @NL80211_RATE_INFO_10_MHZ_WIDTH: 10 MHz width - note that this is
+ *	a legacy rate and will be reported as the actual bitrate, i.e.
+ *	half the base (20 MHz) rate
+ * @NL80211_RATE_INFO_5_MHZ_WIDTH: 5 MHz width - note that this is
+ *	a legacy rate and will be reported as the actual bitrate, i.e.
+ *	a quarter of the base (20 MHz) rate
  * @__NL80211_RATE_INFO_AFTER_LAST: internal use
  */
 enum nl80211_rate_info {
@@ -2127,6 +2315,8 @@ enum nl80211_rate_info {
 	NL80211_RATE_INFO_80_MHZ_WIDTH,
 	NL80211_RATE_INFO_80P80_MHZ_WIDTH,
 	NL80211_RATE_INFO_160_MHZ_WIDTH,
+	NL80211_RATE_INFO_10_MHZ_WIDTH,
+	NL80211_RATE_INFO_5_MHZ_WIDTH,
 
 	/* keep last */
 	__NL80211_RATE_INFO_AFTER_LAST,
@@ -2171,18 +2361,24 @@ enum nl80211_sta_bss_param {
  *
  * @__NL80211_STA_INFO_INVALID: attribute number 0 is reserved
  * @NL80211_STA_INFO_INACTIVE_TIME: time since last activity (u32, msecs)
- * @NL80211_STA_INFO_RX_BYTES: total received bytes (u32, from this station)
- * @NL80211_STA_INFO_TX_BYTES: total transmitted bytes (u32, to this station)
- * @NL80211_STA_INFO_RX_BYTES64: total received bytes (u64, from this station)
- * @NL80211_STA_INFO_TX_BYTES64: total transmitted bytes (u64, to this station)
+ * @NL80211_STA_INFO_RX_BYTES: total received bytes (MPDU length)
+ *	(u32, from this station)
+ * @NL80211_STA_INFO_TX_BYTES: total transmitted bytes (MPDU length)
+ *	(u32, to this station)
+ * @NL80211_STA_INFO_RX_BYTES64: total received bytes (MPDU length)
+ *	(u64, from this station)
+ * @NL80211_STA_INFO_TX_BYTES64: total transmitted bytes (MPDU length)
+ *	(u64, to this station)
  * @NL80211_STA_INFO_SIGNAL: signal strength of last received PPDU (u8, dBm)
  * @NL80211_STA_INFO_TX_BITRATE: current unicast tx rate, nested attribute
  * 	containing info as possible, see &enum nl80211_rate_info
- * @NL80211_STA_INFO_RX_PACKETS: total received packet (u32, from this station)
- * @NL80211_STA_INFO_TX_PACKETS: total transmitted packets (u32, to this
- *	station)
- * @NL80211_STA_INFO_TX_RETRIES: total retries (u32, to this station)
- * @NL80211_STA_INFO_TX_FAILED: total failed packets (u32, to this station)
+ * @NL80211_STA_INFO_RX_PACKETS: total received packet (MSDUs and MMPDUs)
+ *	(u32, from this station)
+ * @NL80211_STA_INFO_TX_PACKETS: total transmitted packets (MSDUs and MMPDUs)
+ *	(u32, to this station)
+ * @NL80211_STA_INFO_TX_RETRIES: total retries (MPDUs) (u32, to this station)
+ * @NL80211_STA_INFO_TX_FAILED: total failed packets (MPDUs)
+ *	(u32, to this station)
  * @NL80211_STA_INFO_SIGNAL_AVG: signal strength average (u8, dBm)
  * @NL80211_STA_INFO_LLID: the station's mesh LLID
  * @NL80211_STA_INFO_PLID: the station's mesh PLID
@@ -2206,6 +2402,16 @@ enum nl80211_sta_bss_param {
  *	Same format as NL80211_STA_INFO_CHAIN_SIGNAL.
  * @NL80211_STA_EXPECTED_THROUGHPUT: expected throughput considering also the
  *	802.11 header (u32, kbps)
+ * @NL80211_STA_INFO_RX_DROP_MISC: RX packets dropped for unspecified reasons
+ *	(u64)
+ * @NL80211_STA_INFO_BEACON_RX: number of beacons received from this peer (u64)
+ * @NL80211_STA_INFO_BEACON_SIGNAL_AVG: signal strength average
+ *	for beacons only (u8, dBm)
+ * @NL80211_STA_INFO_TID_STATS: per-TID statistics (see &enum nl80211_tid_stats)
+ *	This is a nested attribute where each the inner attribute number is the
+ *	TID+1 and the special TID 16 (i.e. value 17) is used for non-QoS frames;
+ *	each one of those is again nested with &enum nl80211_tid_stats
+ *	attributes carrying the actual values.
  * @__NL80211_STA_INFO_AFTER_LAST: internal
  * @NL80211_STA_INFO_MAX: highest possible station info attribute
  */
@@ -2238,6 +2444,10 @@ enum nl80211_sta_info {
 	NL80211_STA_INFO_CHAIN_SIGNAL,
 	NL80211_STA_INFO_CHAIN_SIGNAL_AVG,
 	NL80211_STA_INFO_EXPECTED_THROUGHPUT,
+	NL80211_STA_INFO_RX_DROP_MISC,
+	NL80211_STA_INFO_BEACON_RX,
+	NL80211_STA_INFO_BEACON_SIGNAL_AVG,
+	NL80211_STA_INFO_TID_STATS,
 
 	/* keep last */
 	__NL80211_STA_INFO_AFTER_LAST,
@@ -2245,6 +2455,31 @@ enum nl80211_sta_info {
 };
 
 /**
+ * enum nl80211_tid_stats - per TID statistics attributes
+ * @__NL80211_TID_STATS_INVALID: attribute number 0 is reserved
+ * @NL80211_TID_STATS_RX_MSDU: number of MSDUs received (u64)
+ * @NL80211_TID_STATS_TX_MSDU: number of MSDUs transmitted (or
+ *	attempted to transmit; u64)
+ * @NL80211_TID_STATS_TX_MSDU_RETRIES: number of retries for
+ *	transmitted MSDUs (not counting the first attempt; u64)
+ * @NL80211_TID_STATS_TX_MSDU_FAILED: number of failed transmitted
+ *	MSDUs (u64)
+ * @NUM_NL80211_TID_STATS: number of attributes here
+ * @NL80211_TID_STATS_MAX: highest numbered attribute here
+ */
+enum nl80211_tid_stats {
+	__NL80211_TID_STATS_INVALID,
+	NL80211_TID_STATS_RX_MSDU,
+	NL80211_TID_STATS_TX_MSDU,
+	NL80211_TID_STATS_TX_MSDU_RETRIES,
+	NL80211_TID_STATS_TX_MSDU_FAILED,
+
+	/* keep last */
+	NUM_NL80211_TID_STATS,
+	NL80211_TID_STATS_MAX = NUM_NL80211_TID_STATS - 1
+};
+
+/**
  * enum nl80211_mpath_flags - nl80211 mesh path flags
  *
  * @NL80211_MPATH_FLAG_ACTIVE: the mesh path is active
@@ -2577,6 +2812,11 @@ enum nl80211_sched_scan_match_attr {
  * @NL80211_RRF_AUTO_BW: maximum available bandwidth should be calculated
  *	base on contiguous rules and wider channels will be allowed to cross
  *	multiple contiguous/overlapping frequency ranges.
+ * @NL80211_RRF_GO_CONCURRENT: See &NL80211_FREQUENCY_ATTR_GO_CONCURRENT
+ * @NL80211_RRF_NO_HT40MINUS: channels can't be used in HT40- operation
+ * @NL80211_RRF_NO_HT40PLUS: channels can't be used in HT40+ operation
+ * @NL80211_RRF_NO_80MHZ: 80MHz operation not allowed
+ * @NL80211_RRF_NO_160MHZ: 160MHz operation not allowed
  */
 enum nl80211_reg_rule_flags {
 	NL80211_RRF_NO_OFDM		= 1<<0,
@@ -2589,11 +2829,18 @@ enum nl80211_reg_rule_flags {
 	NL80211_RRF_NO_IR		= 1<<7,
 	__NL80211_RRF_NO_IBSS		= 1<<8,
 	NL80211_RRF_AUTO_BW		= 1<<11,
+	NL80211_RRF_GO_CONCURRENT	= 1<<12,
+	NL80211_RRF_NO_HT40MINUS	= 1<<13,
+	NL80211_RRF_NO_HT40PLUS		= 1<<14,
+	NL80211_RRF_NO_80MHZ		= 1<<15,
+	NL80211_RRF_NO_160MHZ		= 1<<16,
 };
 
 #define NL80211_RRF_PASSIVE_SCAN	NL80211_RRF_NO_IR
 #define NL80211_RRF_NO_IBSS		NL80211_RRF_NO_IR
 #define NL80211_RRF_NO_IR		NL80211_RRF_NO_IR
+#define NL80211_RRF_NO_HT40		(NL80211_RRF_NO_HT40MINUS |\
+					 NL80211_RRF_NO_HT40PLUS)
 
 /* For backport compatibility with older userspace */
 #define NL80211_RRF_NO_IR_ALL		(NL80211_RRF_NO_IR | __NL80211_RRF_NO_IBSS)
@@ -2646,16 +2893,18 @@ enum nl80211_user_reg_hint_type {
  * @NL80211_SURVEY_INFO_FREQUENCY: center frequency of channel
  * @NL80211_SURVEY_INFO_NOISE: noise level of channel (u8, dBm)
  * @NL80211_SURVEY_INFO_IN_USE: channel is currently being used
- * @NL80211_SURVEY_INFO_CHANNEL_TIME: amount of time (in ms) that the radio
- *	spent on this channel
- * @NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY: amount of the time the primary
+ * @NL80211_SURVEY_INFO_TIME: amount of time (in ms) that the radio
+ *	was turned on (on channel or globally)
+ * @NL80211_SURVEY_INFO_TIME_BUSY: amount of the time the primary
  *	channel was sensed busy (either due to activity or energy detect)
- * @NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY: amount of time the extension
+ * @NL80211_SURVEY_INFO_TIME_EXT_BUSY: amount of time the extension
  *	channel was sensed busy
- * @NL80211_SURVEY_INFO_CHANNEL_TIME_RX: amount of time the radio spent
- *	receiving data
- * @NL80211_SURVEY_INFO_CHANNEL_TIME_TX: amount of time the radio spent
- *	transmitting data
+ * @NL80211_SURVEY_INFO_TIME_RX: amount of time the radio spent
+ *	receiving data (on channel or globally)
+ * @NL80211_SURVEY_INFO_TIME_TX: amount of time the radio spent
+ *	transmitting data (on channel or globally)
+ * @NL80211_SURVEY_INFO_TIME_SCAN: time the radio spent for scan
+ *	(on this channel or globally)
  * @NL80211_SURVEY_INFO_MAX: highest survey info attribute number
  *	currently defined
  * @__NL80211_SURVEY_INFO_AFTER_LAST: internal use
@@ -2665,17 +2914,25 @@ enum nl80211_survey_info {
 	NL80211_SURVEY_INFO_FREQUENCY,
 	NL80211_SURVEY_INFO_NOISE,
 	NL80211_SURVEY_INFO_IN_USE,
-	NL80211_SURVEY_INFO_CHANNEL_TIME,
-	NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY,
-	NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY,
-	NL80211_SURVEY_INFO_CHANNEL_TIME_RX,
-	NL80211_SURVEY_INFO_CHANNEL_TIME_TX,
+	NL80211_SURVEY_INFO_TIME,
+	NL80211_SURVEY_INFO_TIME_BUSY,
+	NL80211_SURVEY_INFO_TIME_EXT_BUSY,
+	NL80211_SURVEY_INFO_TIME_RX,
+	NL80211_SURVEY_INFO_TIME_TX,
+	NL80211_SURVEY_INFO_TIME_SCAN,
 
 	/* keep last */
 	__NL80211_SURVEY_INFO_AFTER_LAST,
 	NL80211_SURVEY_INFO_MAX = __NL80211_SURVEY_INFO_AFTER_LAST - 1
 };
 
+/* keep old names for compatibility */
+#define NL80211_SURVEY_INFO_CHANNEL_TIME		NL80211_SURVEY_INFO_TIME
+#define NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY		NL80211_SURVEY_INFO_TIME_BUSY
+#define NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY	NL80211_SURVEY_INFO_TIME_EXT_BUSY
+#define NL80211_SURVEY_INFO_CHANNEL_TIME_RX		NL80211_SURVEY_INFO_TIME_RX
+#define NL80211_SURVEY_INFO_CHANNEL_TIME_TX		NL80211_SURVEY_INFO_TIME_TX
+
 /**
  * enum nl80211_mntr_flags - monitor configuration flags
  *
@@ -3055,14 +3312,20 @@ enum nl80211_bss_scan_width {
  * @NL80211_BSS_BSSID: BSSID of the BSS (6 octets)
  * @NL80211_BSS_FREQUENCY: frequency in MHz (u32)
  * @NL80211_BSS_TSF: TSF of the received probe response/beacon (u64)
+ *	(if @NL80211_BSS_PRESP_DATA is present then this is known to be
+ *	from a probe response, otherwise it may be from the same beacon
+ *	that the NL80211_BSS_BEACON_TSF will be from)
  * @NL80211_BSS_BEACON_INTERVAL: beacon interval of the (I)BSS (u16)
  * @NL80211_BSS_CAPABILITY: capability field (CPU order, u16)
  * @NL80211_BSS_INFORMATION_ELEMENTS: binary attribute containing the
  *	raw information elements from the probe response/beacon (bin);
- *	if the %NL80211_BSS_BEACON_IES attribute is present, the IEs here are
- *	from a Probe Response frame; otherwise they are from a Beacon frame.
+ *	if the %NL80211_BSS_BEACON_IES attribute is present and the data is
+ *	different then the IEs here are from a Probe Response frame; otherwise
+ *	they are from a Beacon frame.
  *	However, if the driver does not indicate the source of the IEs, these
  *	IEs may be from either frame subtype.
+ *	If present, the @NL80211_BSS_PRESP_DATA attribute indicates that the
+ *	data here is known to be from a probe response, without any heuristics.
  * @NL80211_BSS_SIGNAL_MBM: signal strength of probe response/beacon
  *	in mBm (100 * dBm) (s32)
  * @NL80211_BSS_SIGNAL_UNSPEC: signal strength of the probe response/beacon
@@ -3074,6 +3337,10 @@ enum nl80211_bss_scan_width {
  *	yet been received
  * @NL80211_BSS_CHAN_WIDTH: channel width of the control channel
  *	(u32, enum nl80211_bss_scan_width)
+ * @NL80211_BSS_BEACON_TSF: TSF of the last received beacon (u64)
+ *	(not present if no beacon frame has been received yet)
+ * @NL80211_BSS_PRESP_DATA: the data in @NL80211_BSS_INFORMATION_ELEMENTS and
+ *	@NL80211_BSS_TSF is known to be from a probe response (flag attribute)
  * @__NL80211_BSS_AFTER_LAST: internal
  * @NL80211_BSS_MAX: highest BSS attribute
  */
@@ -3091,6 +3358,8 @@ enum nl80211_bss {
 	NL80211_BSS_SEEN_MS_AGO,
 	NL80211_BSS_BEACON_IES,
 	NL80211_BSS_CHAN_WIDTH,
+	NL80211_BSS_BEACON_TSF,
+	NL80211_BSS_PRESP_DATA,
 
 	/* keep last */
 	__NL80211_BSS_AFTER_LAST,
@@ -3100,6 +3369,9 @@ enum nl80211_bss {
 /**
  * enum nl80211_bss_status - BSS "status"
  * @NL80211_BSS_STATUS_AUTHENTICATED: Authenticated with this BSS.
+ *	Note that this is no longer used since cfg80211 no longer
+ *	keeps track of whether or not authentication was done with
+ *	a given BSS.
  * @NL80211_BSS_STATUS_ASSOCIATED: Associated with this BSS.
  * @NL80211_BSS_STATUS_IBSS_JOINED: Joined to this IBSS.
  *
@@ -3313,6 +3585,8 @@ enum nl80211_ps_state {
  *	interval in which %NL80211_ATTR_CQM_TXE_PKTS and
  *	%NL80211_ATTR_CQM_TXE_RATE must be satisfied before generating an
  *	%NL80211_CMD_NOTIFY_CQM. Set to 0 to turn off TX error reporting.
+ * @NL80211_ATTR_CQM_BEACON_LOSS_EVENT: flag attribute that's set in a beacon
+ *	loss event
  * @__NL80211_ATTR_CQM_AFTER_LAST: internal
  * @NL80211_ATTR_CQM_MAX: highest key attribute
  */
@@ -3325,6 +3599,7 @@ enum nl80211_attr_cqm {
 	NL80211_ATTR_CQM_TXE_RATE,
 	NL80211_ATTR_CQM_TXE_PKTS,
 	NL80211_ATTR_CQM_TXE_INTVL,
+	NL80211_ATTR_CQM_BEACON_LOSS_EVENT,
 
 	/* keep last */
 	__NL80211_ATTR_CQM_AFTER_LAST,
@@ -3337,9 +3612,7 @@ enum nl80211_attr_cqm {
  *      configured threshold
  * @NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH: The RSSI is higher than the
  *      configured threshold
- * @NL80211_CQM_RSSI_BEACON_LOSS_EVENT: The device experienced beacon loss.
- *	(Note that deauth/disassoc will still follow if the AP is not
- *	available. This event might get used as roaming event, etc.)
+ * @NL80211_CQM_RSSI_BEACON_LOSS_EVENT: (reserved, never sent)
  */
 enum nl80211_cqm_rssi_threshold_event {
 	NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
@@ -3479,6 +3752,28 @@ struct nl80211_pattern_support {
  * @NL80211_WOWLAN_TRIG_WAKEUP_TCP_NOMORETOKENS: For wakeup reporting only,
  *	the TCP connection ran out of tokens to use for data to send to the
  *	service
+ * @NL80211_WOWLAN_TRIG_NET_DETECT: wake up when a configured network
+ *	is detected.  This is a nested attribute that contains the
+ *	same attributes used with @NL80211_CMD_START_SCHED_SCAN.  It
+ *	specifies how the scan is performed (e.g. the interval, the
+ *	channels to scan and the initial delay) as well as the scan
+ *	results that will trigger a wake (i.e. the matchsets).  This
+ *	attribute is also sent in a response to
+ *	@NL80211_CMD_GET_WIPHY, indicating the number of match sets
+ *	supported by the driver (u32).
+ * @NL80211_WOWLAN_TRIG_NET_DETECT_RESULTS: nested attribute
+ *	containing an array with information about what triggered the
+ *	wake up.  If no elements are present in the array, it means
+ *	that the information is not available.  If more than one
+ *	element is present, it means that more than one match
+ *	occurred.
+ *	Each element in the array is a nested attribute that contains
+ *	one optional %NL80211_ATTR_SSID attribute and one optional
+ *	%NL80211_ATTR_SCAN_FREQUENCIES attribute.  At least one of
+ *	these attributes must be present.  If
+ *	%NL80211_ATTR_SCAN_FREQUENCIES contains more than one
+ *	frequency, it means that the match occurred in more than one
+ *	channel.
  * @NUM_NL80211_WOWLAN_TRIG: number of wake on wireless triggers
  * @MAX_NL80211_WOWLAN_TRIG: highest wowlan trigger attribute number
  *
@@ -3504,6 +3799,8 @@ enum nl80211_wowlan_triggers {
 	NL80211_WOWLAN_TRIG_WAKEUP_TCP_MATCH,
 	NL80211_WOWLAN_TRIG_WAKEUP_TCP_CONNLOST,
 	NL80211_WOWLAN_TRIG_WAKEUP_TCP_NOMORETOKENS,
+	NL80211_WOWLAN_TRIG_NET_DETECT,
+	NL80211_WOWLAN_TRIG_NET_DETECT_RESULTS,
 
 	/* keep last */
 	NUM_NL80211_WOWLAN_TRIG,
@@ -3956,6 +4253,47 @@ enum nl80211_ap_sme_features {
  * @NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE: This driver supports dynamic
  *	channel bandwidth change (e.g., HT 20 <-> 40 MHz channel) during the
  *	lifetime of a BSS.
+ * @NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES: This device adds a DS Parameter
+ *	Set IE to probe requests.
+ * @NL80211_FEATURE_WFA_TPC_IE_IN_PROBES: This device adds a WFA TPC Report IE
+ *	to probe requests.
+ * @NL80211_FEATURE_QUIET: This device, in client mode, supports Quiet Period
+ *	requests sent to it by an AP.
+ * @NL80211_FEATURE_TX_POWER_INSERTION: This device is capable of inserting the
+ *	current tx power value into the TPC Report IE in the spectrum
+ *	management TPC Report action frame, and in the Radio Measurement Link
+ *	Measurement Report action frame.
+ * @NL80211_FEATURE_ACKTO_ESTIMATION: This driver supports dynamic ACK timeout
+ *	estimation (dynack). %NL80211_ATTR_WIPHY_DYN_ACK flag attribute is used
+ *	to enable dynack.
+ * @NL80211_FEATURE_STATIC_SMPS: Device supports static spatial
+ *	multiplexing powersave, ie. can turn off all but one chain
+ *	even on HT connections that should be using more chains.
+ * @NL80211_FEATURE_DYNAMIC_SMPS: Device supports dynamic spatial
+ *	multiplexing powersave, ie. can turn off all but one chain
+ *	and then wake the rest up as required after, for example,
+ *	rts/cts handshake.
+ * @NL80211_FEATURE_SUPPORTS_WMM_ADMISSION: the device supports setting up WMM
+ *	TSPEC sessions (TID aka TSID 0-7) with the %NL80211_CMD_ADD_TX_TS
+ *	command. Standard IEEE 802.11 TSPEC setup is not yet supported, it
+ *	needs to be able to handle Block-Ack agreements and other things.
+ * @NL80211_FEATURE_MAC_ON_CREATE: Device supports configuring
+ *	the vif's MAC address upon creation.
+ *	See 'macaddr' field in the vif_params (cfg80211.h).
+ * @NL80211_FEATURE_TDLS_CHANNEL_SWITCH: Driver supports channel switching when
+ *	operating as a TDLS peer.
+ * @NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR: This device/driver supports using a
+ *	random MAC address during scan (if the device is unassociated); the
+ *	%NL80211_SCAN_FLAG_RANDOM_ADDR flag may be set for scans and the MAC
+ *	address mask/value will be used.
+ * @NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR: This device/driver supports
+ *	using a random MAC address for every scan iteration during scheduled
+ *	scan (while not associated), the %NL80211_SCAN_FLAG_RANDOM_ADDR may
+ *	be set for scheduled scan and the MAC address mask/value will be used.
+ * @NL80211_FEATURE_ND_RANDOM_MAC_ADDR: This device/driver supports using a
+ *	random MAC address for every scan iteration during "net detect", i.e.
+ *	scan in unassociated WoWLAN, the %NL80211_SCAN_FLAG_RANDOM_ADDR may
+ *	be set for scheduled scan and the MAC address mask/value will be used.
  */
 enum nl80211_feature_flags {
 	NL80211_FEATURE_SK_TX_STATUS			= 1 << 0,
@@ -3977,6 +4315,32 @@ enum nl80211_feature_flags {
 	NL80211_FEATURE_USERSPACE_MPM			= 1 << 16,
 	NL80211_FEATURE_ACTIVE_MONITOR			= 1 << 17,
 	NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE	= 1 << 18,
+	NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES	= 1 << 19,
+	NL80211_FEATURE_WFA_TPC_IE_IN_PROBES		= 1 << 20,
+	NL80211_FEATURE_QUIET				= 1 << 21,
+	NL80211_FEATURE_TX_POWER_INSERTION		= 1 << 22,
+	NL80211_FEATURE_ACKTO_ESTIMATION		= 1 << 23,
+	NL80211_FEATURE_STATIC_SMPS			= 1 << 24,
+	NL80211_FEATURE_DYNAMIC_SMPS			= 1 << 25,
+	NL80211_FEATURE_SUPPORTS_WMM_ADMISSION		= 1 << 26,
+	NL80211_FEATURE_MAC_ON_CREATE			= 1 << 27,
+	NL80211_FEATURE_TDLS_CHANNEL_SWITCH		= 1 << 28,
+	NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR		= 1 << 29,
+	NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR	= 1 << 30,
+	NL80211_FEATURE_ND_RANDOM_MAC_ADDR		= 1 << 31,
+};
+
+/**
+ * enum nl80211_ext_feature_index - bit index of extended features.
+ *
+ * @NUM_NL80211_EXT_FEATURES: number of extended features.
+ * @MAX_NL80211_EXT_FEATURES: highest extended feature index.
+ */
+enum nl80211_ext_feature_index {
+
+	/* add new features before the definition below */
+	NUM_NL80211_EXT_FEATURES,
+	MAX_NL80211_EXT_FEATURES = NUM_NL80211_EXT_FEATURES - 1
 };
 
 /**
@@ -4025,11 +4389,21 @@ enum nl80211_connect_failed_reason {
  *	dangerous because will destroy stations performance as a lot of frames
  *	will be lost while scanning off-channel, therefore it must be used only
  *	when really needed
+ * @NL80211_SCAN_FLAG_RANDOM_ADDR: use a random MAC address for this scan (or
+ *	for scheduled scan: a different one for every scan iteration). When the
+ *	flag is set, depending on device capabilities the @NL80211_ATTR_MAC and
+ *	@NL80211_ATTR_MAC_MASK attributes may also be given in which case only
+ *	the masked bits will be preserved from the MAC address and the remainder
+ *	randomised. If the attributes are not given full randomisation (46 bits,
+ *	locally administered 1, multicast 0) is assumed.
+ *	This flag must not be requested when the feature isn't supported, check
+ *	the nl80211 feature flags for the device.
  */
 enum nl80211_scan_flags {
 	NL80211_SCAN_FLAG_LOW_PRIORITY			= 1<<0,
 	NL80211_SCAN_FLAG_FLUSH				= 1<<1,
 	NL80211_SCAN_FLAG_AP				= 1<<2,
+	NL80211_SCAN_FLAG_RANDOM_ADDR			= 1<<3,
 };
 
 /**
@@ -4051,6 +4425,25 @@ enum nl80211_acl_policy {
 };
 
 /**
+ * enum nl80211_smps_mode - SMPS mode
+ *
+ * Requested SMPS mode (for AP mode)
+ *
+ * @NL80211_SMPS_OFF: SMPS off (use all antennas).
+ * @NL80211_SMPS_STATIC: static SMPS (use a single antenna)
+ * @NL80211_SMPS_DYNAMIC: dynamic smps (start with a single antenna and
+ *	turn on other antennas after CTS/RTS).
+ */
+enum nl80211_smps_mode {
+	NL80211_SMPS_OFF,
+	NL80211_SMPS_STATIC,
+	NL80211_SMPS_DYNAMIC,
+
+	__NL80211_SMPS_AFTER_LAST,
+	NL80211_SMPS_MAX = __NL80211_SMPS_AFTER_LAST - 1
+};
+
+/**
  * enum nl80211_radar_event - type of radar event for DFS operation
  *
  * Type of event to be used with NL80211_ATTR_RADAR_EVENT to inform userspace
