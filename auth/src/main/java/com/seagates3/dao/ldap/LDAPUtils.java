/*
 * COPYRIGHT 2015 SEAGATE LLC
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF SEAGATE TECHNOLOGY
 * LIMITED, ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF SEAGATE TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF SEAGATE LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF SEAGATE'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A SEAGATE REPRESENTATIVE
 * http://www.seagate.com/contact
 *
 * Original author:  Arjun Hariharan <arjun.hariharan@seagate.com>
 * Original creation date: 17-Sep-2015
 */
package com.seagates3.dao.ldap;

import com.novell.ldap.LDAPConnection;
import com.novell.ldap.LDAPEntry;
import com.novell.ldap.LDAPException;
import com.novell.ldap.LDAPModification;
import com.novell.ldap.LDAPSearchResults;
import java.util.ArrayList;

public class LDAPUtils {

    public static final String BASE_DN = "dc=s3,dc=seagate,dc=com";
    public static final String ACCESS_KEY_ID = "ak";
    public static final String ACCESS_KEY_OBJECT_CLASS = "accesskey";
    public static final String ACCOUNT_ID = "accountid";
    public static final String ACCOUNT_OBJECT_CLASS = "account";
    public static final String ACCOUNT_OU = "accounts";
    public static final String CANONICAL_ID = "canonicalId";
    public static final String CERTIFICATE = "cacertificate;binary";
    public static final String COMMON_NAME = "cn";
    public static final String CREATE_TIMESTAMP = "createtimestamp";
    public static final String DESCRIPTION = "description";
    public static final String DEFAULT_VERSION_ID = "defaultVersionId";
    public static final String EMAIL = "mail";
    public static final String EXPIRY = "exp";
    public static final String FED_ACCESS_KEY_OBJECT_CLASS = "fedaccessKey";
    public static final String GROUP_ID = "groupId";
    public static final String GROUP_NAME = "groupName";
    public static final String GROUP_OBJECT_CLASS = "group";
    public static final String GROUP_OU = "groups";
    public static final String IAMUSER_OBJECT_CLASS = "iamuser";
    public static final String IDP_OU = "idp";
    public static final String ISSUER = "issuer";
    public static final String IS_POLICY_ATTACHABLE = "isPolicyAttachable";
    public static final String MODIFY_TIMESTAMP = "modifyTimestamp";
    public static final String NAME = "name";
    public static final String OBJECT_CLASS = "objectclass";
    public static final String ORGANIZATIONAL_NAME = "o";
    public static final String ORGANIZATIONAL_UNIT_CLASS = "organizationalunit";
    public static final String ORGANIZATIONAL_UNIT_NAME = "ou";
    public static final String PATH = "path";
    public static final String POLICY_DOC = "policyDoc";
    public static final String POLICY_ID = "policyId";
    public static final String POLICY_NAME = "policyName";
    public static final String POLICY_OBJECT_CLASS = "policy";
    public static final String POLICY_OU = "policies";
    public static final String ROLE_NAME = "rolename";
    public static final String ROLE_ID = "roleId";
    public static final String ROLE_OBJECT_CLASS = "role";
    public static final String ROLE_OU = "roles";
    public static final String ROLE_POLICY_DOC = "rolepolicydoc";
    public static final String SAML_METADATA_XML = "samlmetadataxml";
    public static final String SAML_PROVIDER_OBJECT_CLASS = "samlprovider";
    public static final String SAML_TOKENS_JSON = "samltokensjson";
    public static final String SECRET_KEY = "sk";
    public static final String STATUS = "status";
    public static final String TOKEN = "token";
    public static final String USER_ID = "s3userid";
    public static final String USER_OU = "users";

    public static String getBaseDN() {
        return BASE_DN;
    }

    /**
     * Search for an entry in LDAP and return the search results.
     *
     * @param base LDAP Base DN.
     * @param scope LDAP search scope.
     * @param filter LDAP Query filter.
     * @param attrs Attributes to fetch.
     * @return LDAP Search Result.
     * @throws com.novell.ldap.LDAPException
     */
    public static LDAPSearchResults search(String base, int scope,
            String filter, String[] attrs) throws LDAPException {
        LDAPConnection lc;
        lc = LdapConnectionManager.getConnection();
        LDAPSearchResults ldapSearchResult = null;

        if (lc != null && lc.isConnected()) {
            try {
                ldapSearchResult = lc.search(BASE_DN, scope,
                        filter, attrs, false);
            } finally {
                LdapConnectionManager.releaseConnection(lc);
            }
        }

        return ldapSearchResult;
    }

    /**
     * Add a new entry into LDAP.
     *
     * @param newEntry New Entry
     * @throws com.novell.ldap.LDAPException
     */
    public static void add(LDAPEntry newEntry) throws LDAPException {
        LDAPConnection lc;
        lc = LdapConnectionManager.getConnection();

        if (lc != null && lc.isConnected()) {
            try {
                lc.add(newEntry);
            } finally {
                LdapConnectionManager.releaseConnection(lc);
            }
        }
    }

    /**
     * Delete an entry from LDAP.
     *
     * @param dn Distinguished name of the entry.
     * @throws com.novell.ldap.LDAPException
     */
    public static void delete(String dn) throws LDAPException {
        LDAPConnection lc;
        lc = LdapConnectionManager.getConnection();

        if (lc != null && lc.isConnected()) {
            try {
                lc.delete(dn);
            } finally {
                LdapConnectionManager.releaseConnection(lc);
            }
        }
    }

    /**
     * Modify an entry in LDAP
     *
     * @param dn Distinguished name of the entry.
     * @param modification LDAP modification.
     * @throws com.novell.ldap.LDAPException
     */
    public static void modify(String dn, LDAPModification modification)
            throws LDAPException {
        LDAPConnection lc;
        lc = LdapConnectionManager.getConnection();

        if (lc != null && lc.isConnected()) {
            try {
                lc.modify(dn, modification);
            } finally {
                LdapConnectionManager.releaseConnection(lc);
            }
        }
    }

    /**
     * Modify an entry in LDAP
     *
     * @param dn Distinguished name of the entry.
     * @param modList Modification list.
     * @throws com.novell.ldap.LDAPException
     */
    public static void modify(String dn, ArrayList modList) throws LDAPException {
        LDAPConnection lc;
        lc = LdapConnectionManager.getConnection();

        LDAPModification[] modifications = new LDAPModification[modList.size()];
        modifications = (LDAPModification[]) modList.toArray(modifications);

        if (lc != null && lc.isConnected()) {
            try {
                lc.modify(dn, modifications);
            } finally {
                LdapConnectionManager.releaseConnection(lc);
            }
        }
    }
}