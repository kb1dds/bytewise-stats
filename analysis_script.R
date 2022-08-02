# Script to analyze sliding windowed file data

library(tidyverse)

data <- read_csv('~/bytewise-stats/pdf_windows.csv')

data_kmeans <- data %>% 
  select(-byte_offset) %>% 
  kmeans(10)

kmeans_results <- tibble(byte_offset=data$byte_offset,
                         cluster=data_kmeans$cluster) %>%
  left_join(data,by=c(byte_offset='byte_offset'))

kmeans_results %>% 
  pivot_longer(cols=!byte_offset&!cluster,names_to='byte',values_to='count') %>%
  summarize(max(count),min(count),mean(count),sd(count),median(count))

kmeans_results %>% 
  pivot_longer(cols=!byte_offset&!cluster,names_to='byte',values_to='count') %>%
  mutate(byte=as.numeric(byte))%>%
  ggplot(aes(x=byte_offset,y=byte,fill=hsv(cluster/10,1,if_else(count>10,1,count/10))))+
  geom_raster(show.legend = FALSE) +
  scale_fill_identity()
