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
  let { data: monthlyStats } = await api.stats.monthly(1);
  let { data: dailyStats } = await api.stats.daily(1);
  let { data: totals } = await api.stats.totals();
  let { data: monthlyRev } = await api.stats.revenue.monthly(1);
  let { data: dailyRev } = await api.stats.revenue.daily(1);
  let { data: venueRev } = await api.stats.revenue.byVenues(1);

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
    // Растеж събития
    const months1 = toMonths(monthlyStats.events_growth)
    const event_m_count = monthlyStats.events_growth.map((x) => x.count);
    fillBarChart("events-chart-monthly", months1, event_m_count, "Събития", true);

    const days1 = toDays(dailyStats.events_growth)
    const event_d_count = dailyStats.events_growth.map((x) => x.count);
    fillBarChart("events-chart-daily", days1, event_d_count, "Събития", true);

    // Растеж потребители
    if(user.role == 0) {
      const months2 = toMonths(monthlyStats.users_growth)
      const user_m_count = monthlyStats.users_growth.map((x) => x.count);
      fillBarChart("users-chart-monthly", months2, user_m_count, "Потребители", true);

      const days2 = toDays(dailyStats.users_growth)
      const user_d_count = dailyStats.users_growth.map((x) => x.count);
      fillBarChart("users-chart-daily", days2, user_d_count, "Потребители", true);
    }

    // Растеж билети
    const months3 = toMonths(monthlyStats.tickets_growth);
    const ticket_m_count = monthlyStats.tickets_growth.map((x) => x.count);
    fillBarChart("tickets-chart-monthly", months3, ticket_m_count, "Билети", true);

    const days3 = toDays(dailyStats.tickets_growth);
    const ticket_d_count = dailyStats.tickets_growth.map((x) => x.count);
    fillBarChart("tickets-chart-daily", days3, ticket_d_count, "Билети", true);

    // Приходи общи
    const monthly_revenues = monthlyRev.revenue.map((x) => x.revenue);
    fillBarChart("revenue-chart-monthly", months3, monthly_revenues, "Месечен приход (€)");
    const daily_revenues = dailyRev.revenue.map((x) => x.revenue);
    fillBarChart("revenue-chart-daily", days3, daily_revenues, "Дневен приход (€)");

    // Приходи по зали
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

    createMultiLineChart("revenue-venue-chart-monthly", months4, datasets);

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

    if(name === "months1" || name === "months2")
    switch (value) {
      case "1":
        $(`input[name="months1"][value="1"]`).checked = true;
        $(`input[name="months2"][value="1"]`).checked = true;
        await getStats(1);
        break;
      case "3":
        $(`input[name="months1"][value="3"]`).checked = true;
        $(`input[name="months2"][value="3"]`).checked = true;
        await getStats(3);
        break;
      case "6":
        $(`input[name="months1"][value="6"]`).checked = true;
        $(`input[name="months2"][value="6"]`).checked = true;
        await getStats(6);
        break;
    }
    fillCharts();
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

  async function getStats(m) {
    ({ data: monthlyStats } = await api.stats.monthly(m));
    ({ data: dailyStats } = await api.stats.daily(m));
    ({ data: monthlyRev } = await api.stats.revenue.monthly(m));
    ({ data: dailyRev } = await api.stats.revenue.daily(m));
    ({ data: venueRev } = await api.stats.revenue.byVenues(m));
  }
}
