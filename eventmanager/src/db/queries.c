#include "queries.h"
#include "event_queries.h"
#include "ticket_queries.h"
#include "venue_queries.h"
#include "user_queries.h"
#include "../util.h"

static int prepare_query(PGconn* db, const char* name, const char* sql, int nparams) {
    PGresult* res = PQprepare(db, name, sql, nparams, NULL);
    CHECK_COMMAND_QUERY(res, db, 0);
    PQclear(res);
    return 1;
}

int prepare_queries(PGconn* db)
{
    if (!prepare_query(db, "get_events", SQL_GET_EVENTS, 0))
        return 0;
    if (!prepare_query(db, "get_uploaded_events", SQL_GET_UPLOADED_EVENTS, 1))
        return 0;
    if (!prepare_query(db, "get_venue_events", SQL_GET_VENUE_EVENTS, 1))
        return 0;
    if (!prepare_query(db, "get_event", SQL_GET_EVENT, 1))
        return 0;
    if (!prepare_query(db, "add_event", SQL_ADD_EVENT, 6))
        return 0;
    if (!prepare_query(db, "add_event_sectors", SQL_ADD_EVENT_SECTORS, 4))
        return 0;
    if (!prepare_query(db, "update_event_title", SQL_UPDATE_EVENT_TITLE, 2))
        return 0;
    if (!prepare_query(db, "update_event_begins_at", SQL_UPDATE_EVENT_BEGINS_AT, 2))
        return 0;
    if (!prepare_query(db, "update_event_description", SQL_UPDATE_EVENT_DESCRIPTION, 2))
        return 0;
    if (!prepare_query(db, "admin_update_event_image", SQL_ADMIN_UPDATE_EVENT_IMAGE, 2))
        return 0;
    if (!prepare_query(db, "get_event_image_path", SQL_GET_EVENT_IMAGE_PATH, 1))
        return 0;
    if (!prepare_query(db, "activate_event", SQL_ACTIVATE_EVENT, 1))
        return 0;
    if (!prepare_query(db, "deactivate_event", SQL_DEACTIVATE_EVENT, 1))
        return 0;
    if (!prepare_query(db, "get_categories", SQL_GET_CATEGORIES, 1))
        return 0;

    if (!prepare_query(db, "get_event_seatmap", SQL_GET_EVENT_SEATMAP, 1))
        return 0;
    if (!prepare_query(db, "get_venue_seatmap", SQL_GET_VENUE_SEATMAP, 1))
        return 0;
    if (!prepare_query(db, "event_has_seatmap", SQL_EVENT_HAS_SEATMAP, 1))
        return 0;
    if (!prepare_query(db, "venue_has_seatmap", SQL_EVENT_HAS_SEATMAP, 1))
        return 0;
    if (!prepare_query(db, "event_no_seatmap", SQL_EVENT_NO_SEATMAP, 1))
        return 0;
    if (!prepare_query(db, "venue_no_seatmap", SQL_VENUE_NO_SEATMAP, 1))
        return 0;

    if (!prepare_query(db, "check_seat", SQL_CHECK_SEAT, 2))
        return 0;
    if (!prepare_query(db, "get_sector_price", SQL_SECTOR_PRICE, 1))
        return 0;
    if (!prepare_query(db, "add_ticket_user", SQL_ADD_TICKET_USER, 8))
        return 0;
    if (!prepare_query(db, "add_ticket_guest", SQL_ADD_TICKET_GUEST, 7))
        return 0;
    if (!prepare_query(db, "get_tickets", SQL_GET_TICKETS, 1))
        return 0;
    if (!prepare_query(db, "get_ticket", SQL_GET_TICKET, 1))
        return 0;
    if (!prepare_query(db, "get_user_tickets", SQL_GET_USER_TICKETS, 1))
        return 0;
    if (!prepare_query(db, "activate_ticket", SQL_ACTIVATE_TICKET, 1))
        return 0;
    if (!prepare_query(db, "deactivate_ticket", SQL_DEACTIVATE_TICKET, 1))
        return 0;
    if (!prepare_query(db, "ticket_belongs_to_user_id", SQL_TICKET_BELONGS_TO_USER_BY_ID, 1))
        return 0;
    if (!prepare_query(db, "ticket_belongs_to_user_uuid", SQL_TICKET_BELONGS_TO_USER_BY_UUID, 1))
        return 0;

    if (!prepare_query(db, "get_venues", SQL_GET_VENUES, 1))
        return 0;
    if (!prepare_query(db, "get_venue", SQL_GET_VENUE, 1))
        return 0;
    if (!prepare_query(db, "get_sectors", SQL_GET_SECTORS, 1))
        return 0;
    if (!prepare_query(db, "get_cities", SQL_GET_CITIES, 0))
        return 0;
    if (!prepare_query(db, "add_venue", SQL_ADD_VENUE, 3))
        return 0;
    if (!prepare_query(db, "add_venue_sector", SQL_ADD_VENUE_SECTOR, 1))
        return 0;
    if (!prepare_query(db, "update_venue_name", SQL_UPDATE_VENUE_NAME, 2))
        return 0;
    if (!prepare_query(db, "update_venue_address", SQL_UPDATE_VENUE_ADDRESS, 2))
        return 0;
    if (!prepare_query(db, "deactivate_venue", SQL_DEACTIVATE_VENUE, 1))
        return 0;
    if (!prepare_query(db, "activate_venue", SQL_ACTIVATE_VENUE, 1))
        return 0;

    if (!prepare_query(db, "get_users", SQL_GET_USERS, 0))
        return 0;
    if (!prepare_query(db, "get_user_by_id", SQL_GET_USER_BY_ID, 1))
        return 0;
    if (!prepare_query(db, "get_user_by_email", SQL_GET_USER_BY_EMAIL, 1))
        return 0;
    if (!prepare_query(db, "get_user_password", SQL_GET_USER_PASSWORD, 1))
        return 0;
    if (!prepare_query(db, "verify_password", SQL_VERIFY_PASSWORD, 1))
        return 0;
    if (!prepare_query(db, "verify_email", SQL_VERIFY_EMAIL, 1))
        return 0;
    if (!prepare_query(db, "check_role", SQL_CHECK_ROLE, 1))
        return 0;
    if (!prepare_query(db, "add_user", SQL_ADD_USER, 7))
        return 0;
    if (!prepare_query(db, "update_role", SQL_UPDATE_ROLE, 2))
        return 0;
    if (!prepare_query(db, "update_names", SQL_UPDATE_NAMES, 3))
        return 0;
    if (!prepare_query(db, "update_password", SQL_UPDATE_PASSWORD, 3))
        return 0;
    if (!prepare_query(db, "update_email", SQL_UPDATE_EMAIL, 2))
        return 0;
    if (!prepare_query(db, "update_phone", SQL_UPDATE_PHONE, 2))
        return 0;
    if (!prepare_query(db, "soft_delete_user", SQL_SOFT_DELETE_USER, 1))
        return 0;
    if (!prepare_query(db, "restore_user", SQL_RESTORE_USER, 1))
        return 0;
    if (!prepare_query(db, "activate_user", SQL_ACTIVATE_USER, 1))
        return 0;
    if (!prepare_query(db, "deactivate_user", SQL_DEACTIVATE_USER, 1))
        return 0;
    if (!prepare_query(db, "permanent_delete_user", SQL_PERMANENT_DELETE_USER, 1))
        return 0;
    if (!prepare_query(db, "permanent_delete_users", SQL_PERMANENT_DELETE_USERS, 0))
        return 0;
    if (!prepare_query(db, "create_reset_token", SQL_CREATE_RESET_TOKEN, 2))
        return 0;
    if (!prepare_query(db, "validate_reset_token", SQL_VALIDATE_RESET_TOKEN, 1))
        return 0;
    if (!prepare_query(db, "delete_reset_token", SQL_DELETE_RESET_TOKEN, 1))
        return 0;
    if (!prepare_query(db, "delete_reset_tokens", SQL_DELETE_RESET_TOKENS, 0))
        return 0;

    if (!prepare_query(db, "get_total_events", SQL_TOTAL_EVENTS, 1))
        return 0;
    if (!prepare_query(db, "get_total_venues", SQL_TOTAL_VENUES, 0))
        return 0;
    if (!prepare_query(db, "get_total_tickets", SQL_TOTAL_TICKETS, 1))
        return 0;
    if (!prepare_query(db, "get_total_users", SQL_TOTAL_USERS, 0))
        return 0;

    if (!prepare_query(db, "get_users_growth_monthly", SQL_USERS_GROWTH_MONTHLY, 1))
        return 0;
    if (!prepare_query(db, "get_users_growth_daily", SQL_USERS_GROWTH_DAILY, 1))
        return 0;
    if (!prepare_query(db, "get_events_growth_monthly", SQL_EVENTS_GROWTH_MONTHLY, 2))
        return 0;
    if (!prepare_query(db, "get_events_growth_daily", SQL_EVENTS_GROWTH_DAILY, 2))
        return 0;
    if (!prepare_query(db, "get_tickets_growth_monthly", SQL_TICKETS_GROWTH_MONTHLY, 2))
        return 0;
    if (!prepare_query(db, "get_tickets_growth_daily", SQL_TICKETS_GROWTH_DAILY, 2))
        return 0;

    if (!prepare_query(db, "get_total_revenue", SQL_TOTAL_REVENUE, 1))
        return 0;
    if (!prepare_query(db, "get_revenue_daily", SQL_REVENUE_DAILY, 2))
        return 0;
    if (!prepare_query(db, "get_revenue_monthly", SQL_REVENUE_MONTHLY, 2))
        return 0;
    if (!prepare_query(db, "get_revenue_by_venue_monthly", SQL_REVENUE_BY_VENUE_MONTHLY, 2))
        return 0;

    return 1;
}
