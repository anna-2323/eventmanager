#pragma once

const char* SQL_GET_USERS =
"SELECT id, email, first_name, last_name, phone, role, deleted_on, active "
"FROM data.users; ";

const char* SQL_GET_USER_BY_ID =
"SELECT id, email, first_name, last_name, phone, role, deleted_on, active "
"FROM data.users "
"WHERE id = $1; ";

const char* SQL_GET_USER_BY_EMAIL =
"SELECT id, email, first_name, last_name, phone, role, deleted_on "
"FROM data.users "
"WHERE email = $1; ";

const char* SQL_GET_USER_PASSWORD =
"SELECT password_hash, salt "
"FROM data.users "
"WHERE id = $1 "
"AND active;";

const char* SQL_VERIFY_PASSWORD =
"SELECT password_hash, salt FROM data.users WHERE id = $1";

const char* SQL_VERIFY_EMAIL =
"SELECT id FROM data.users WHERE email = $1";

const char* SQL_CHECK_ROLE =
"SELECT role "
"FROM data.users "
"WHERE id = $1; ";

const char* SQL_ADD_USER =
"INSERT INTO data.users (first_name, last_name, email, phone, password_hash, salt, role) "
"VALUES ($1, $2, $3, NULLIF($4, ''), $5, $6, $7) "
"RETURNING id";

const char* SQL_UPDATE_ROLE =
"UPDATE data.users SET role = $1 WHERE id = $2";

const char* SQL_UPDATE_NAMES =
"UPDATE data.users SET first_name = $1, last_name = $2 WHERE id = $3";

const char* SQL_UPDATE_PASSWORD =
"UPDATE data.users SET password_hash = $1, salt = $2 WHERE id = $3";

const char* SQL_UPDATE_EMAIL =
"UPDATE data.users SET email = $1 WHERE id = $2";

const char* SQL_UPDATE_PHONE =
"UPDATE data.users SET phone = $1 WHERE id = $2";

const char* SQL_SOFT_DELETE_USER =
"UPDATE data.users SET deleted_on = NOW() WHERE id = $1";

const char* SQL_RESTORE_USER =
"UPDATE data.users SET deleted_on = NULL WHERE id = $1;";

const char* SQL_ACTIVATE_USER =
"UPDATE data.users SET active = TRUE WHERE id = $1";

const char* SQL_DEACTIVATE_USER =
"UPDATE data.users SET active = FALSE WHERE id = $1";

const char* SQL_PERMANENT_DELETE_USER =
"DELETE FROM data.users "
"WHERE id = $1 ";

const char* SQL_PERMANENT_DELETE_USERS =
"DELETE FROM data.users "
"WHERE deleted_on IS NOT NULL "
"AND deleted_on < NOW() - INTERVAL '30 days'";

const char* SQL_CREATE_RESET_TOKEN =
"INSERT INTO data.password_resets (token, user_id) VALUES ($1, $2)";

const char* SQL_VALIDATE_RESET_TOKEN =
"SELECT user_id FROM data.password_resets "
"WHERE token = $1 AND expires_at > NOW()";

const char* SQL_DELETE_RESET_TOKEN =
"DELETE FROM data.password_resets WHERE token = $1";

const char* SQL_DELETE_RESET_TOKENS =
"DELETE FROM data.password_resets WHERE expires_at < NOW()";

const char* SQL_TOTAL_USERS =
"SELECT COUNT(*) "
"FROM data.users "
"WHERE deleted_on IS NULL "
"AND active = TRUE "
"AND active = true;";

const char* SQL_USERS_GROWTH_MONTHLY =
"SELECT DATE_TRUNC('month', u.created_at) AS month, "
"COUNT(*) AS registrations "
"FROM data.users u "
"WHERE u.created_at >= CURRENT_DATE - INTERVAL '12 months' "
"AND u.active = TRUE "
"GROUP BY DATE_TRUNC('month', u.created_at) "
"ORDER BY month; ";

const char* SQL_USERS_GROWTH_MONTHLY_ALL =
"SELECT DATE_TRUNC('month', u.created_at) AS month, "
"COUNT(*) AS registrations "
"FROM data.users u "
"WHERE u.active = TRUE "
"GROUP BY DATE_TRUNC('month', u.created_at) "
"ORDER BY month; ";

const char* SQL_USERS_GROWTH_DAILY =
"SELECT DATE_TRUNC('day', u.created_at) AS day, "
"COUNT(*) AS registrations "
"FROM data.users u "
"WHERE u.created_at >= CURRENT_DATE - INTERVAL '30 days' "
"AND u.active = TRUE "
"GROUP BY DATE_TRUNC('day', u.created_at) "
"ORDER BY day; ";
