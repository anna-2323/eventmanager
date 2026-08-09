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
                                <td><span id="user-role" class="tag is-warning">
                                        ${u.role == 2 ? "Потребител" : u.role == 1 ? "Организатор" : "Администратор"}
                                    </span>
                                </td>
                            </tr>
                            <tr><th>Статус</th>
                                <td><span id="user-status" class="tag is-success">
                                        ${u.active ? "Активен" : "Деактивиран"}
                                    </span>
                                </td>
                            </tr>
                            <tr>
                                <th>За изтриване</th>
                                <td><span id="user-delete" class="tag is-light">
                                        ${u.deleted_on ? `${toDate(u.deleted_on)}` : "Не"}
                                    </span>
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
                        <button class="button is-warning" id="deactivate-btn">
                            Деактивирай
                        </button>
                        <button class="button is-danger" id="delete-btn">
                            Изтрий
                        </button>
                    </div>
                </div>
            </div>`;
}

export function eventCard(e) {
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
                            <tr><th>Одобрено</th>
                                <td id="event-verified">${e.verified ? 
                                    "<span style='color:#00d1b2;'>Да</span>" : 
                                    "<span style='color:#ff6685;'>Не</span>"}</td>
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
                        <button class="button is-warning" id="verify-btn">
                            ${e.verified ? 'Отмени одобряване' : 'Одобри'}
                        </button>
                        <button class="button is-danger" id="delete-btn">
                            Изтрий
                        </button>
                    </div>
                </div>
            </div>`;
}