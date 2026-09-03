import { toDate } from "../core/dom.js";

export function userCard(u) {
  return `<div class="box">
                    <h2 class="subtitle">Информация</h2>

                    <table class="table is-fullwidth">
                        <tbody>
                            <tr><th>ID</th>
                                <td id="user-id">${u.id}</td>
                            </tr>
                            <tr><th>Име</th>
                                <td id="user-name">${u.first_name + " " + u.last_name}</td>
                            </tr>
                            <tr><th>Имейл</th>
                                <td id="user-email">${u.email}</td>
                            </tr>
                            <tr><th>Телефон</th>
                                <td id="user-phone">${u.phone}</td>
                            </tr>
                            <tr><th>Роля</th>
                                <td>
                                    ${u.role == 2 ? "Потребител" : u.role == 1 ? "Организатор" : "Администратор"}
                                </td>
                            </tr>
                            <tr><th>Статус</th>
                                <td>
                                    ${u.active ? 
                                        "<span style='color:#00d1b2;'>Активeн</span>" : 
                                        "<span style='color:#ff6685;'>Деактивиран</span>"
                                    }
                                </td>
                            </tr>
                            <tr>
                                <th>За изтриване</th>
                                <td>${u.deleted_on ?
                                            `<span style='color:#ff6685;'>${toDate(u.deleted_on)}</span>` :
                                            "Не"}
                                </td>
                            </tr>
                        </tbody>
                    </table>


                    <div class="buttons mt-4">
                        <button class="button is-primary" id="events-btn">
                            Събития
                        </button>
                        <button class="button is-info" id="edit-btn">
                            Редактирай
                        </button>
                        <button class="button is-warning" id="activate-btn">
                            ${u.active ? "Деактивирай" : "Активирай"}
                        </button>
                        <button class="button is-danger" id="delete-btn">
                            Изтрий
                        </button>
                    </div>
                </div>
            </div>`;
}

export function eventCard(e, role) {
    return `<div class="box">
                    <h2 class="subtitle">Информация</h2>

                    <table class="table is-fullwidth">
                        <tbody>
                            <tr><th>ID</th>
                                <td id="event-id">${e.id}</td>
                            </tr>
                            <tr><th>Име</th>
                                <td id="event-title">${e.title}</td>
                            </tr>
                            <tr><th>Време</th>
                                <td id="event-begins-at">${toDate(e.begins_at)}</td>
                            </tr>
                            <tr><th>Локация</th>
                                <td id="event-venue-name">${e.venue_name}, ${e.city}</td>
                            </tr>
                            <tr><th>Останали места</th>
                                <td id="event-seats-left">${e.seats_left}</td>
                            </tr>
                            <tr><th>Статус</th>
                                <td id="event-verified">${e.active ? 
                                    "<span style='color:#00d1b2;'>Активно</span>" : 
                                    "<span style='color:#ff6685;'>Деактивирано</span>"}</td>
                            </tr>
                        </tbody>
                    </table>


                    <div class="buttons mt-4">
                        <button class="button is-primary" id="layout-btn">
                            Преглед на зала
                        </button>
                        <button class="button is-info" id="edit-btn">
                            Редактирай
                        </button>
                        ${role == 0 ? 
                            `<button class="button is-warning" id="activate-btn">
                                ${e.active ? 'Деактивирай' : 'Активирай'}
                            </button>` : ''
                        }
                    </div>
                </div>
            </div>`;
}

export function venueCard(v, role) {
    return `<div class="box">
                    <h2 class="subtitle">Информация</h2>

                    <table class="table is-fullwidth">
                        <tbody>
                            <tr><th>ID</th>
                                <td id="event-id">${v.id}</td>
                            </tr>
                            <tr><th>Име</th>
                                <td id="event-title">${v.venue_name}</td>
                            </tr>
                            <tr><th>Град</th>
                                <td id="event-begins-at">${v.city}</td>
                            </tr>
                            <tr><th>Адрес</th>
                                <td id="event-venue-name">${v.address}</td>
                            </tr>
                            <tr><th>Статус</th>
                                <td id="event-verified">${v.active ? 
                                    "<span style='color:#00d1b2;'>Активна</span>" : 
                                    "<span style='color:#ff6685;'>Деактивирана</span>"}</td>
                            </tr>
                        </tbody>
                    </table>


                    <div class="buttons mt-4">
                        <button class="button is-primary" id="events-btn">
                            Преглед на събития
                        </button>
                        <button class="button is-info" id="edit-btn">
                            Редактирай
                        </button>
                        ${role == 0 ? 
                            `<button class="button is-warning" id="activate-btn">
                                ${v.active ? 'Деактивирай' : 'Активирай'}
                            </button>` : ''
                        }
                    </div>
                </div>
            </div>`;
}

export function ticketCard(t, role) {
    return `<div class="box">
                    <h2 class="subtitle">Информация</h2>

                    <table class="table is-fullwidth">
                        <tbody>
                            <tr><th>ID</th>
                                <td id="ticket-id">${t.id}</td>
                            </tr>
                            <tr><th>Име</th>
                                <td id="ticket-names">${t.first_name} ${t.last_name}</td>
                            </tr>
                            <tr><th>Събитие</th>
                                <td id="ticket-event">${t.event_name}</td>
                            </tr>
                            <tr><th>Зала</th>
                                <td id="ticket-venue">${t.venue_name}, гр. ${t.venue_city}</td>
                            </tr>
                            <tr><th>Сектор</th>
                                <td id="ticket-sector">${t.sector_name ? t.sector_name : "-"}</td>
                            </tr>
                            <tr><th>Статус</th>
                                <td id="ticket-verified">${t.active ? 
                                    "<span style='color:#00d1b2;'>Активен</span>" : 
                                    "<span style='color:#ff6685;'>Деактивиран</span>"}</td>
                            </tr>
                        </tbody>
                    </table>


                    <div class="buttons mt-4">
                        <a href="/admin/events/${t.event_id}"><button class="button is-primary" id="event-btn">
                            Преглед на събитие
                        </button></a>
                        ${(role == 0 && t.user_id) ? 
                        `<a href="/admin/users/${t.user_id}"><button class="button is-primary" id="event-btn">
                            Преглед на потребител
                        </button></a>` : ''
                        }
                        <button class="button is-warning" id="activate-btn">
                            ${t.active ? 'Деактивирай' : 'Активирай'}
                        </button>
                    </div>
                </div>
            </div>`;
}