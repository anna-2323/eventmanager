const BASE_URL = '/api';

async function request(path = {}, options = {}) {
  const res = await fetch(BASE_URL + path, {
    headers: { 'Content-Type': 'application/json', ...options.headers },
    ...options,
    credentials: 'include',
  });
  if (!res.ok) throw new Error(`${await res.text()}`);
  return res.status === 204 ? null : res.json();
}

async function create(path, data, method) {
    const res = await fetch(BASE_URL + path, {
        method: method,
        body: data
    });

    return await res.json();
}

export const api = {
  auth: {
    getUser: () => request('/me'),
    login: (data) => request('/login', { method: 'POST', body: JSON.stringify(data) }),
    logout: () => request('/logout', { method: 'GET' }),
    signup: (data) => request('/signup', { method: 'POST', body: JSON.stringify(data) }),
    forgotPassword: (email) => request('/forgot', { method: 'POST', body: JSON.stringify({ email }) }),
    resetPassword: (data) => request('/reset', { method: 'POST', body: JSON.stringify(data) }),
  },

  cities: {
    list: () => request('/cities')
  },

  categories: {
    list: () => request('/categories')
  },
  
  venues: {
    list: () => request('/venues'),
    get: (id) => request(`/venues/${id}`),
    getEvents: (id) => request(`/venues/${id}/events`)
  },

  events: {
    list: (params) => request('/events' + toQuery(params)),
    get: (id) => request(`/events/${id}`),
    getSeatMap: (id) => request(`/events/seatmap/${id}`)
  },

  tickets: {
    purchase: (id, data) => request(`/purchase/${id}`, { method: 'POST', body: JSON.stringify(data) }),
    confirm: (token) => request(`/confirmation/${token}`),
    getMine: () => request(`/mytickets`)
  },

  users: {
    edit: (data) => request('/profile', { method: 'PATCH', body: JSON.stringify(data) }),
    delete: (data) => request('/profile', { method: 'DELETE', body: JSON.stringify(data) })
  },

  stats: {
    monthly: () => request('/stats/monthly'),
    daily: () => request('/stats/daily'),
    totals: () => request('/stats/totals'),
    revenue: {
      monthly: () => request('/stats/revenue/monthly'),
      daily: () => request('/stats/revenue/daily'),
      byVenues: () => request('/stats/revenue/venues')
    },
    // export: {
    //   users: () =>  request('/stats/export/users'),
    //   events: () => request('/stats/export/events'),
    //   tickets: () => request('/stats/export/tickets'),
    //   revenueMonthly: () => request('/stats/export/revenue'),
    //   revenueVenues: () => request('/stats/export/revenue/venues')
    // }
  },

  admin: {
    users: {
      list: () => request('/admin/users'),
      get: (id) => request(`/admin/users/${id}`),
      events: (id) => request(`/admin/users/${id}/events`),
      edit: (id, data) => request(`/admin/users/${id}`, { method: 'PATCH', body: JSON.stringify(data) }),
      delete: (id) => request(`/admin/users/${id}`, { method: 'DELETE' })
    },
    events: {
      list: () => request('/admin/events'),
      get: (id) => request(`/admin/events/${id}`),
      create: (data) => create('/admin/events', data, "POST"),
      edit: (id, data) => request(`/admin/events/${id}`, { method: 'PATCH', body: JSON.stringify(data) }),
      editImage: (id, data) => create(`/admin/events/${id}`, data, "PATCH"),
      delete: (id) => request(`/admin/events/${id}`, { method: 'DELETE' })
    },
    venues: {
      create: (data) => request('/admin/venues', { method: 'POST', body: JSON.stringify(data) }),
      edit: (id, data) => request(`/admin/venues/${id}`, { method: 'PATCH', body: JSON.stringify(data) })
    },
    tickets: {
      list: () => request('/admin/tickets'),
      get: (id) => request(`/admin/tickets/${id}`),
      edit: (id, data) => request(`/admin/tickets/${id}`, {method: 'PATCH', body: JSON.stringify(data)})
    }
  },
};

function toQuery(params) {
  if (!params) return '';

  const filtered = Object.entries(params)
    .filter(([, value]) => value !== null && value !== undefined && value !== '');

  if (filtered.length === 0) {
    return '';
  }

  return '?' + new URLSearchParams(filtered).toString();
}
