/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2015 MonetDB B.V.
 * All Rights Reserved.
 */

import java.sql.*;
import nl.cwi.monetdb.jdbc.types.*;

public class Test_PSsqldata {
	public static void main(String[] args) throws Exception {
		Class.forName("nl.cwi.monetdb.jdbc.MonetDriver");
		Connection con = DriverManager.getConnection(args[0]);
		Statement stmt = con.createStatement();
		PreparedStatement pstmt;
		ResultSet rs = null;
		ResultSetMetaData rsmd = null;
		ParameterMetaData pmd = null;

		con.setAutoCommit(false);
		// >> false: auto commit should be off now
		System.out.println("0. false\t" + con.getAutoCommit());

		try {
			stmt.executeUpdate("CREATE TABLE table_Test_PSsqldata ( myinet inet, myurl url )");

			pstmt = con.prepareStatement("INSERT INTO table_Test_PSsqldata VALUES (?, ?)");

			pmd = pstmt.getParameterMetaData();
			System.out.println("1. 2 parameters:\t" + pmd.getParameterCount());
			for (int col = 1; col <= pmd.getParameterCount(); col++) {
				System.out.println("" + col + ".");
				System.out.println("\ttype          " + pmd.getParameterType(col));
				System.out.println("\ttypename      " + pmd.getParameterTypeName(col));
				System.out.println("\tclassname     " + pmd.getParameterClassName(col));
			}

			INET tinet = new INET();
			URL turl = new URL();

			tinet.fromString("172.5.5.5/24");
			turl.fromString("http://www.monetdb.org/");
			pstmt.setObject(1, tinet);
			pstmt.setObject(2, turl);
			pstmt.execute();

			tinet.setNetmaskBits(16);
			pstmt.execute();

			rs = stmt.executeQuery("SELECT * FROM table_Test_PSsqldata");
			rsmd = rs.getMetaData();

			for (int i = 1; rs.next(); i++) {
				for (int col = 1; col <= rsmd.getColumnCount(); col++) {
					Object x = rs.getObject(col);
					if (x == null) {
						System.out.println("" + i + ".\t<null>");
					} else {
						System.out.println("" + i + ".\t" + x.toString());
						if (x instanceof INET) {
							INET inet = (INET)x;
							System.out.println("\t" + inet.getAddress() + "/" + inet.getNetmaskBits());
							System.out.println("\t" + inet.getInetAddress().toString());
						} else if (x instanceof URL) {
							URL url = (URL)x;
							System.out.println("\t" + url.getURL().toString());
						}
					}
				}
			}
		} catch (SQLException e) {
			System.out.println("failed :( "+ e.getMessage());
			System.out.println("ABORTING TEST!!!");
		}

		con.rollback();
		con.close();
	}
}
