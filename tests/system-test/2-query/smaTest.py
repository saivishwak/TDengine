###################################################################
#           Copyright (c) 2016 by TAOS Technologies, Inc.
#                     All rights reserved.
#
#  This file is proprietary and confidential to TAOS Technologies.
#  No part of this file may be reproduced, stored, transmitted,
#  disclosed or used in any form or by any means other than as
#  expressly provided by the written permission from Jianhui Tao
#
###################################################################

# -*- coding: utf-8 -*-

import sys

from numpy.lib.function_base import insert
import taos
from util.log import *
from util.cases import *
from util.sql import *
import numpy as np

# constant define
WAITS = 5 # wait seconds

class TDTestCase:
    #
    # --------------- main frame -------------------
    #
    
    def caseDescription(self):
        '''
        limit and offset keyword function test cases;
        case1: limit offset base function test
        case2: offset return valid
        '''
        return 

    # init
    def init(self, conn, logSql):
        tdLog.debug("start to execute %s" % __file__)
        tdSql.init(conn.cursor())
        tdSql.prepare()
        self.create_tables();
        self.ts = 1500000000000


    # run case  
    def run(self):
        # insert data
        self.insert_data("t1", self.ts, 1000*10000)
        self.insert_data1("t4", self.ts, 1000*10000)        
        # test base case
        # self.test_case1()
        tdLog.debug(" LIMIT test_case1 ............ [OK]")
        # test advance case
        # self.test_case2()
        tdLog.debug(" LIMIT test_case2 ............ [OK]")


    # stop 
    def stop(self):
        tdSql.close()
        tdLog.success("%s successfully executed" % __file__)

    #
    # --------------- case  -------------------
    #

    # create table
    def create_tables(self):
        # super table
        tdSql.execute("create table st(ts timestamp, i1 int) tags(area int)");
        # child table
        tdSql.execute("create table t1 using st tags(1)");
        tdSql.execute("create table t2 using st tags(2)");
        tdSql.execute("create table t3 using st tags(3)");

        tdSql.execute("create table st1(ts timestamp, i1 int ,i2 int) tags(area int) sma(i2) ");
        tdSql.execute("create table t4 using st1 tags(1)");

        return 

    # insert data1
    def insert_data(self, tbname, ts_start, count):
        pre_insert = "insert into %s values"%tbname
        sql = pre_insert
        tdLog.debug("doing insert table %s rows=%d ..."%(tbname, count))
        for i in range(count):
            sql += " (%d,%d)"%(ts_start + i*1000, i )
            if i >0 and i%30000 == 0:
                tdSql.execute(sql)
                sql = pre_insert
        # end sql        
        if sql != pre_insert:
            tdSql.execute(sql)

        tdLog.debug("INSERT TABLE DATA ............ [OK]")
        return

    def insert_data1(self, tbname, ts_start, count):
        pre_insert = "insert into %s values"%tbname
        sql = pre_insert
        tdLog.debug("doing insert table %s rows=%d ..."%(tbname, count))
        for i in range(count):
            sql += " (%d,%d,%d)"%(ts_start + i*1000, i , i+1)
            if i >0 and i%30000 == 0:
                tdSql.execute(sql)
                sql = pre_insert
        # end sql        
        if sql != pre_insert:
            tdSql.execute(sql)

        tdLog.debug("INSERT TABLE DATA ............ [OK]")
        return

    # test case1 base 
    # def test_case1(self):
    #     # 
    #     # limit base function
    #     #
    #     # base no where
    #     sql = "select * from t1 limit 10"
    #     tdSql.waitedQuery(sql, 10, WAITS)
    #     tdSql.checkData(0, 1, 0)
    #     tdSql.checkData(9, 1, 9)

    #     # have where
    #     sql = "select * from t1 where ts>='2017-07-14 10:40:01' and ts<'2017-07-14 10:40:06' limit 10"
    #     tdSql.waitedQuery(sql, 5, WAITS)
    #     tdSql.checkData(0, 1, 1)
    #     tdSql.checkData(4, 1, 5)

    #     # 
    #     # offset base function
    #     #
    #     # no where
    #     sql = "select * from t1 limit 10 offset 5"
    #     tdSql.waitedQuery(sql, 10, WAITS)
    #     tdSql.checkData(0, 1, 5)
    #     tdSql.checkData(9, 1, 14)

    #     # have where only ts
    #     sql = "select * from t1 where ts>='2017-07-14 10:40:10' and ts<'2017-07-14 10:40:20' limit 10 offset 5"
    #     tdSql.waitedQuery(sql, 5, WAITS)
    #     tdSql.checkData(0, 1, 15)
    #     tdSql.checkData(4, 1, 19)

    #     # have where with other column condition
    #     sql = "select * from t1 where i1>=1 and i1<11 limit 10 offset 5"
    #     tdSql.waitedQuery(sql, 5, WAITS)
    #     tdSql.checkData(0, 1, 6)
    #     tdSql.checkData(4, 1, 10)

    #     # have where with ts and other column condition
    #     sql = "select * from t1 where ts>='2017-07-14 10:40:10' and ts<'2017-07-14 10:40:50' and i1>=20 and i1<=25 limit 10 offset 5"
    #     tdSql.waitedQuery(sql, 1, WAITS)
    #     tdSql.checkData(0, 1, 25)

    #     return 

    # # test advance 
    # def test_case2(self):
    #     # 
    #     # OFFSET merge file data with memory data
    #     #
        
    #     # offset
    #     sql = "select * from t1 limit 10 offset 72000"
    #     tdSql.waitedQuery(sql, 10, WAITS)
    #     tdSql.checkData(0, 1, 72000)

    #     # each insert one row into NO.0 NO.2 NO.7 blocks
    #     sql = "insert into t1 values (%d, 0) (%d, 2) (%d, 7)"%(self.ts+1, self.ts + 2*3300*1000+1, self.ts + 7*3300*1000+1)
    #     tdSql.execute(sql)
    #     # query result
    #     sql = "select * from t1 limit 10 offset 72000"
    #     tdSql.waitedQuery(sql, 10, WAITS)
    #     tdSql.checkData(0, 1, 72000 - 3)
        
    #     # have where
    #     sql = "select * from t1 where ts>='2017-07-14 10:40:10' and ts<'2017-07-22 18:40:10' limit 10 offset 72000"
    #     tdSql.waitedQuery(sql, 10, WAITS)
    #     tdSql.checkData(0, 1, 72000 - 3 + 10 + 1)

#
# add case with filename
#
tdCases.addWindows(__file__, TDTestCase())
tdCases.addLinux(__file__, TDTestCase())