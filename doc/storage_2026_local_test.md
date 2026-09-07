# 2026 client storage compatibility

Storage packet guards use `PACKETVER_MAIN_NUM >= 20260715`, based on the
kRO MAIN release of numbered storage tabs on July 15, 2026:
https://ro.gnjoy.com/news/notice/View.asp?BBSMode=10001&seq=8277
The notice was posted July 13; July 15 is the maintenance/deployment date.
It explicitly describes numbered tabs, a switching cooldown and no extra
opening fee when switching. This is separate from the later fourth storage.
This gate is release-date-based, not proof of the earliest executable build:
no July predecessor/successor packet dumps were available to establish that
boundary. Packet layouts were verified on the local September client.
The global `PACKETVER` selection is unchanged.

Targets local PACKETVER 20260902. Modern storage uses 0x0c71 for tab counts,
0x0c73 to open the selected tab, and 0x0c72 for tab selection. Item counts
continue to use 0x00f2. Existing item-list and inventory-end packets remain.

Local test tabs are restricted to group ID 99 and map UI 1/2/3 to storage IDs
0/1/2. Configure the two additional IDs in conf/import/inter_server.yml:

```yaml
Header:
  Type: INTER_SERVER_DB
  Version: 1
Body:
  - ID: 1
    Name: "Test Storage 2"
    Table: storage_test_2
    Max: 600
  - ID: 2
    Name: "Test Storage 3"
    Table: storage_test_3
    Max: 600
```

After verifying the target tables do not exist, create empty tables using
`CREATE TABLE storage_test_2 LIKE storage;` and
`CREATE TABLE storage_test_3 LIKE storage;`. Existing storage rows are untouched.
Restart char/map servers with players logged out to load the configuration.

Dirty tab changes save through the existing storage close path and wait for
the matching char-server acknowledgement. Repeated clicks do not replace the
pending target. Failed saves reopen the previous cache without discarding items.

Validation: Release x64 build; old/modern packet table compilation; standalone
tests of production switch functions with I/O doubles covering successful,
failed, unrelated and stale ACKs, repeated clicks, invalid IDs, and clean tabs.
User confirmed in-game operation after the automatic-switch deployment.

Client message resources also need the modern storage keys
MSI_STORE_TITLEKAPRA through MSI_STORE_NOTNUMBER. These are supplied separately
in the local ui-storage-messages-20260906.grf overlay; this repository does not
contain client binaries/GRFs. The protocol changes do not implement paid emoji.

Rollback: restore the prior map binary and configuration with players logged
out. Retain extra storage tables until deposited items have been recovered;
do not drop tables as part of an automatic rollback.
