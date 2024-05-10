/*
 Navicat Premium Data Transfer

 Source Server         : Inspection
 Source Server Type    : SQLite
 Source Server Version : 3035005 (3.35.5)
 Source Schema         : main

 Target Server Type    : SQLite
 Target Server Version : 3035005 (3.35.5)
 File Encoding         : 65001

 Date: 15/08/2023 18:35:09
*/

PRAGMA foreign_keys = false;

-- ----------------------------
-- Table structure for drone
-- ----------------------------
DROP TABLE IF EXISTS "drone";
CREATE TABLE "drone" (
  "droneID" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "snCode" text NOT NULL,
  "lensType" text,
  "maxSpeed" text,
  "systemVersion" text,
  "videoAddr" text,
  "status" integer,
  "droneType" TEXT,
  "liveAddr" TEXT
);

-- ----------------------------
-- Auto increment value for drone
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 23 WHERE name = 'drone';

PRAGMA foreign_keys = true;
