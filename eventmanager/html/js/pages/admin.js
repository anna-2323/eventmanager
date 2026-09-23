import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { createMultiLineChart, createBarChart } from "../components/graph.js";

header();

const { data: user } = await api.auth.getUser();
if (!user.logged_in || user.role == 2) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  const { data: monthlyStats } = await api.stats.monthly();
  const { data: dailyStats } = await api.stats.daily();
  const { data: totals } = await api.stats.totals();
  const { data: monthlyRev } = await api.stats.revenue.monthly();
  const { data: venueRev } = await api.stats.revenue.byVenues();

  const monthNames = [
    "Яну",
    "Фев",
    "Мар",
    "Апр",
    "Май",
    "Юни",
    "Юли",
    "Авг",
    "Сеп",
    "Окт",
    "Ное",
    "Дек",
  ];

  fillTotalValues();
  fillCharts(monthlyStats);
  setupTabs();

  function fillTotalValues() {
    if(user.role == 0) {
      $("#total-users").innerHTML = totals.users;
      $("#total-venues").innerHTML = totals.venues;
    }
    $("#total-events").innerHTML = totals.events;
    $("#total-tickets").innerHTML = totals.tickets;
  }

  function fillBarChart(id, timePeriods, count, name, integers = false) {
    createBarChart(id, timePeriods, count, name, integers);
  }

  function fillCharts() {
    const months1 = toMonths(monthlyStats.events_growth)
    const event_count = monthlyStats.events_growth.map((x) => x.count);
    fillBarChart("events-chart", months1, event_count, "Събития", true);

    if(user.role == 0) {
      const months2 = toMonths(monthlyStats.users_growth)
      const user_count = monthlyStats.users_growth.map((x) => x.count);
      fillBarChart("users-chart", months2, user_count, "Потребители", true);
    }

    const months3 = toMonths(monthlyStats.tickets_growth)
    const ticket_count = monthlyStats.tickets_growth.map((x) => x.count);
    fillBarChart("tickets-chart", months3, ticket_count, "Билети", true);

    const revenues = monthlyRev.revenue.map((x) => x.revenue);
    fillBarChart("revenue-chart", months3, revenues, "Месечен приход (€)");

    const months4 = [...new Set(venueRev.revenue.map((x) => x.period))].sort();
    const venueIds = [...new Set(venueRev.revenue.map((x) => x.venue_id))];

    const datasets = venueIds.map((venueId) => ({
      label: `Зала ${venueId}`,
      data: months4.map((month) => {
        const entry = venueRev.revenue.find(
          (x) => x.venue_id === venueId && x.period === month,
        );

        return entry ? entry.revenue : 0;
      }),
    }));

    createMultiLineChart("revenue-venue-chart", months4, datasets);

  }

  function setupTabs() {
    const tabs = document.querySelectorAll(".tabs li[data-tab]");
    const contents = document.querySelectorAll("#tab-content > div");
  
    tabs.forEach(tab => {
        tab.addEventListener("click", () => {
            const target = tab.dataset.tab;
        
            // Активира се избрания раздел
            tabs.forEach(t => {
                t.classList.toggle("is-active", t === tab);
            });
          
            // Показва се избраното съдържание
            contents.forEach(content => {
                content.classList.toggle(
                    "is-hidden",
                    content.id !== target
                );
            });
        });
    });
  }

  document.addEventListener("change", async (event) => {
    if (event.target.type !== "radio") return;

    const { name, value } = event.target;

    switch (name) {
      case "event-growth":
        if (value === "monthly") {
          // събития помесечно
          const months = toMonths(monthlyStats.events_growth);
          const event_count = monthlyStats.events_growth.map((x) => x.count);
          fillBarChart("events-chart", months, event_count, "Събития", true);
        } else {
          // събития подневно
          const days = toDays(dailyStats.events_growth);
          const event_count = dailyStats.events_growth.map((x) => x.count);
          fillBarChart("events-chart", days, event_count, "Събития", true);
        }
        break;

      case "user-growth":
        if (value === "monthly") {
          // потребители помесечно
          const months = toMonths(monthlyStats.users_growth);
          const user_count = monthlyStats.users_growth.map((x) => x.count);
          fillBarChart("users-chart", months, user_count, "Потребители", true);
        } else {
          // потребители подневно
          const days = toDays(dailyStats.events_growth);
          const user_count = dailyStats.users_growth.map((x) => x.count);
          fillBarChart("users-chart", days, user_count, "Потребители", true);
        }
        break;

      case "ticket-growth":
        if (value === "monthly") {
          // билети помесечно
          const months = toMonths(monthlyStats.tickets_growth);
          const ticket_count = monthlyStats.tickets_growth.map(
            (x) => x.ticket_count,
          );
          fillBarChart("tickets-chart", months, ticket_count, "Билети", true);
        } else {
          // билети подневно
          const days = toDays(dailyStats.tickets_growth);
          const ticket_count = dailyStats.tickets_growth.map((x) => x.count);
          fillBarChart("tickets-chart", days, ticket_count, "Билети", true);
        }
        break;
    }
  });

  function toMonths(stats) {
    return stats.map((item) => {
      const month = Number(item.period.split("-")[1]);
      return monthNames[month - 1];
    });
  }

  function toDays(stats) {
    return stats.map((item) => {
      const date = new Date(item.period);
      return date.toLocaleDateString("bg-BG", {
        day: "numeric",
        month: "short",
      });
    });
  }
}
