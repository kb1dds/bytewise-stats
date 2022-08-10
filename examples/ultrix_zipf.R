# Do Ultrix-11 bytes follow a Zipf law?

library(tidyverse)

data <- read_csv('~/bytewise-stats/examples/20220802_ultrix_rl_13.dump.csv')

# Memory snapshot
data %>% 
  pivot_longer(cols=!byte_offset,names_to='byte',values_to='count') %>%
  mutate(byte=as.numeric(byte))%>%
  ggplot(aes(x=byte_offset,y=byte,fill=hsv(0,0,if_else(count>10,1,count/10)))) +
  geom_raster(show.legend = FALSE) +
  scale_fill_identity() +
  scale_y_continuous(breaks=c(0,32,127,255)) +
  scale_x_continuous(breaks=seq(0,2^18,2^15))#,limits=c(0,2^16))

# Zipf's law plot... if it's really following Zipf's law, 
# this should be linear with slope -1... It's not!
data %>% 
  summarize(across(.cols=!byte_offset,sum)) %>%
  pivot_longer(cols=everything(), names_to='byte',values_to='count') %>%
  filter(byte!=0) %>%
  arrange(desc(count)) %>% 
  mutate(position=row_number()) %>%
  ggplot(aes(x=log10(position),y=log10(count))) +
  geom_line() + geom_point() + 
  geom_smooth(method='lm') +
  xlab('Log position in sorted list of byte values') +
  ylab('Log frequency')

# Byte values sorted by frequency
data %>% 
  summarize(across(.cols=!byte_offset,sum)) %>%
  pivot_longer(cols=everything(), names_to='byte',values_to='count') %>%
  arrange(desc(count)) %>%
  mutate(octal=map(byte,~sprintf('%o',as.numeric(.)))) %>%
  View()
