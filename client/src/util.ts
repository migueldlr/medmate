export const formatTime = (t: string): string => {
  let hour = Number(t.slice(0, 2));
  const minute = t.slice(2, 4);
  const pm = hour > 12;

  hour = hour % 12 === 0 ? 12 : hour % 12;

  return `${hour}:${minute}${pm ? 'PM' : 'AM'}`;
};

export const formatTimeList = (times: string[]): string => {
  switch (times.length) {
    case 0:
      return '';
    case 1:
      return formatTime(times[0]);
    case 2:
      return `${formatTime(times[0])} and ${formatTime(times[1])}`;
    default:
      return (
        times.slice(0, -1).reduce((acc, time) => {
          return acc + `${formatTime(time)}, `;
        }, '') + `and ${formatTime(times[times.length - 1])}`
      );
  }
};
